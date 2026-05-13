#include "evrp/client/api/evrp.h"

#include <functional>
#include <memory>
#include <string>

#include "evrp/client/impl/playback.h"
#include "evrp/client/impl/record.h"
#include "evrp/device/api/client.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"

namespace evrp::client {
namespace {

int connectDeviceAndRun(
    MemorySetting settings,
    const std::function<int(MemorySetting&&, Ioc&)>& body) {
  logging::LogLevel logLevel =
      settings.get("logLevel", logging::LogLevel::Info);
  logService->setLevel(logLevel);

  std::string device = settings.get<std::string>("device", {});
  std::unique_ptr<evrp::device::api::IClient> deviceClient =
      evrp::device::api::makeClient(device, settings);
  if (!deviceClient) {
    logError(
        "Could not connect to evrp-device{} (session handshake failed). "
        "Start `evrp-device`, set --device=HOST:PORT, or rely on UDP broadcast "
        "discovery (default when --device is unset; see --discovery_port and "
        "--discovery_link_mode).",
        device.empty() ? std::string("") : (" at " + device));
    return 1;
  }
  settings.insert("device", deviceClient->serverAddress());

  Ioc ioc;
  ioc.emplace(deviceClient->playback());
  ioc.emplace(deviceClient->inputListener());

  std::unique_ptr<IEnhancedFileSystem> enhancedFs(
      createEnhancedFileSystem(createFileSystem()));
  ioc.emplace(enhancedFs.get());

  return body(std::move(settings), ioc);
}

}  // namespace

int runRecord(MemorySetting settings) {
  return connectDeviceAndRun(std::move(settings),
                             [](MemorySetting&& s, Ioc& ioc) {
                               return Record(std::move(s), ioc).run();
                             });
}

int runReplay(MemorySetting settings) {
  std::string playbackPath =
      settings.get<std::string>("playbackPath", {});
  if (playbackPath.empty()) {
    logError("Replay requires playbackPath in settings.");
    return 1;
  }
  return connectDeviceAndRun(std::move(settings),
                             [](MemorySetting&& s, Ioc& ioc) {
                               return Playback(std::move(s), ioc).run();
                             });
}

}  // namespace evrp::client
