#include "evrp/server/impl/server/localevrp.h"

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "evrp/server/impl/server/grpc/grpcsettingkey.h"
#include "evrp/server/impl/server/playback.h"
#include "evrp/server/impl/server/record.h"
#include "evrp/device/api/client.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"
#include "evrp/device/impl/client/client_session.h"
#include "evrp/device/impl/client/remotelogservice.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/scopeguard.h"
#include "evrp/sdk/setting/isetting.h"

namespace {

struct DeviceLogForwarder {
  std::atomic<bool> stop{false};
  std::thread th;

  DeviceLogForwarder() = default;
  DeviceLogForwarder(const DeviceLogForwarder&) = delete;
  DeviceLogForwarder& operator=(const DeviceLogForwarder&) = delete;

  ~DeviceLogForwarder() {
    stop.store(true, std::memory_order_release);
    if (th.joinable()) {
      th.join();
    }
  }
};

struct ConnectedClient {
  std::unique_ptr<evrp::device::api::IClient> deviceClient;
  std::unique_ptr<IEnhancedFileSystem> enhancedFs;
  evrp::Ioc ioc;
  std::shared_ptr<ISetting> settings;
  std::unique_ptr<DeviceLogForwarder> deviceLogForward;
};

std::optional<ConnectedClient> connectDevice(
    std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return std::nullopt;
  }

  logging::LogLevel logLevel =
      settings->get("logLevel", logging::LogLevel::Info);
  logService->setLevel(logLevel);

  std::string device = settings->get<std::string>("device", {});
  std::unique_ptr<evrp::device::api::IClient> deviceClient =
      evrp::device::api::makeClient(device, *settings);
  if (!deviceClient) {
    logError(
        "Could not connect to evrp-device{} (session handshake failed). "
        "Start `evrp-device`, set --device=HOST:PORT, or rely on UDP broadcast "
        "discovery (default when --device is unset; see --discovery_port and "
        "--discovery_link_mode).",
        device.empty() ? std::string("") : (" at " + device));
    return std::nullopt;
  }
  settings->insert("device", deviceClient->serverAddress());

  ConnectedClient out;
  out.deviceClient = std::move(deviceClient);
  out.enhancedFs = std::unique_ptr<IEnhancedFileSystem>(
      createEnhancedFileSystem(createFileSystem()));
  out.ioc.emplace(out.deviceClient->playback());
  out.ioc.emplace(out.deviceClient->inputListener());
  out.ioc.emplace(out.enhancedFs.get());
  out.settings = std::move(settings);

  out.deviceLogForward = std::make_unique<DeviceLogForwarder>();
  DeviceLogForwarder* fw = out.deviceLogForward.get();
  std::shared_ptr<grpc::Channel> logCh;
  std::string sid;
  if (!evrp::device::impl::tryExportSessionForLogForwarding(
          out.deviceClient.get(), &logCh, &sid)) {
    logError(
        "Device log forwarding: session export failed (unexpected IClient "
        "implementation); device logs will not stream to the server.");
  } else {
    fw->th = std::thread([logCh, sid, stop = &fw->stop]() {
      evrp::device::client::RemoteLogService logService(logCh, sid);
      logService.forwardUntil(stop);
    });
  }
  return out;
}

}  // namespace

namespace evrp::server {

int LocalEvrp::record(std::shared_ptr<ISetting> settings) {
  auto connected = connectDevice(std::move(settings));
  if (!connected) {
    return 1;
  }
  ConnectedClient c = std::move(*connected);
  grpc::ServerContext* rpcCtx =
      c.settings->get<grpc::ServerContext*>(kUnaryRpcServerContextSettingKey,
                                            nullptr);
  evrp::sdk::ScopeGuard endRecording([&]() {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    activeListener_ = nullptr;
    isRecording_.store(false);
    stopRecordingRequested_.store(false);
  });
  stopRecordingRequested_.store(false);
  isRecording_.store(true);
  {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    activeListener_ = c.ioc.get<evrp::device::api::IInputListener>();
  }
  Record rec(std::move(c.settings), c.ioc);
  rec.setExternalCancelFlag(&stopRecordingRequested_);
  if (rpcCtx != nullptr) {
    rec.setCancelPredicate(
        [rpcCtx]() { return rpcCtx->IsCancelled(); });
  }
  return rec.run();
}

int LocalEvrp::replay(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  std::string playbackPath =
      settings->get<std::string>("playbackPath", {});
  if (playbackPath.empty()) {
    logError("Replay requires playbackPath in settings.");
    return 1;
  }
  auto connected = connectDevice(std::move(settings));
  if (!connected) {
    return 1;
  }
  ConnectedClient c = std::move(*connected);
  grpc::ServerContext* rpcCtx =
      c.settings->get<grpc::ServerContext*>(kUnaryRpcServerContextSettingKey,
                                            nullptr);
  evrp::sdk::ScopeGuard endReplay([&]() {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    activePlayback_ = nullptr;
    isReplaying_.store(false);
  });
  isReplaying_.store(true);
  {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    activePlayback_ = c.ioc.get<evrp::device::api::IPlayback>();
  }

  std::atomic<bool> rpcCancelWatchDone{false};
  std::thread rpcCancelWatch;
  if (rpcCtx != nullptr) {
    rpcCancelWatch = std::thread([this, rpcCtx, &rpcCancelWatchDone]() {
      while (!rpcCancelWatchDone.load(std::memory_order_acquire)) {
        if (rpcCtx->IsCancelled()) {
          stopReplay();
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
  }
  evrp::sdk::ScopeGuard joinRpcCancelWatch([&]() {
    rpcCancelWatchDone.store(true, std::memory_order_release);
    if (rpcCancelWatch.joinable()) {
      rpcCancelWatch.join();
    }
  });

  return Playback(std::move(c.settings), c.ioc).run();
}

bool LocalEvrp::isRecording() const {
  return isRecording_.load(std::memory_order_acquire);
}

bool LocalEvrp::isReplaying() const {
  return isReplaying_.load(std::memory_order_acquire);
}

bool LocalEvrp::stopRecording() {
  stopRecordingRequested_.store(true, std::memory_order_release);
  std::lock_guard<std::mutex> lock(sessionMutex_);
  if (activeListener_) {
    activeListener_->cancelListening();
  }
  return true;
}

bool LocalEvrp::stopReplay() {
  std::lock_guard<std::mutex> lock(sessionMutex_);
  if (!activePlayback_) {
    return true;
  }
  return activePlayback_->stopPlayback();
}

}  // namespace evrp::server
