#include "evrp/server/impl/server/localevrp.h"

#include <memory>
#include <optional>
#include <string>

#include "evrp/server/impl/server/playback.h"
#include "evrp/server/impl/server/record.h"
#include "evrp/device/api/client.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/scopeguard.h"
#include "evrp/sdk/setting/isetting.h"

namespace {

struct ConnectedClient {
  std::unique_ptr<evrp::device::api::IClient> deviceClient;
  std::unique_ptr<IEnhancedFileSystem> enhancedFs;
  evrp::Ioc ioc;
  std::shared_ptr<ISetting> settings;
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
