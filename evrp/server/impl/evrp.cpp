#include "evrp/server/api/evrp.h"

#include <memory>
#include <optional>
#include <string>

#include "evrp/server/impl/playback.h"
#include "evrp/server/impl/record.h"
#include "evrp/device/api/client.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"
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

int record(std::shared_ptr<ISetting> settings) {
  auto connected = connectDevice(std::move(settings));
  if (!connected) {
    return 1;
  }
  ConnectedClient c = std::move(*connected);
  return Record(std::move(c.settings), c.ioc).run();
}

int replay(std::shared_ptr<ISetting> settings) {
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
  return Playback(std::move(c.settings), c.ioc).run();
}

}  // namespace evrp::server
