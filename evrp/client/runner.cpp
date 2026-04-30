#include "evrp/client/runner.h"

#include "evrp/client/argparser.h"
#include "evrp/client/playback.h"
#include "evrp/client/record.h"
#include "evrp/device/api/client.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"

#include <memory>

Runner::Runner(MapStringKeyStoreCore options)
    : options_(std::move(options)),
      optionsView_(options_),
      prog_(optionsView_.get<std::string>("program", "evrp")),
      recording_(optionsView_.get<bool>("recording", false)),
      playback_(optionsView_.get<bool>("playback", false)),
      device_(optionsView_.get<std::string>("device", {})),
      playbackPath_(optionsView_.get<std::string>("playbackPath", {})),
      logLevel_(optionsView_.get("logLevel", logging::LogLevel::Info)) {}

int Runner::run() {
  logService->setLevel(logLevel_);

  int modeCount = (recording_ ? 1 : 0) + (playback_ ? 1 : 0);
  if (modeCount > 1) {
    logError("Cannot use --record and --playback at the same time.");
    printUsage(prog_.c_str());
    return 1;
  }

  if (modeCount == 0) {
    printUsage(prog_.c_str());
    return 1;
  }

  std::unique_ptr<evrp::device::api::IClient> deviceClient =
      evrp::device::api::makeClient(device_);
  if (!deviceClient) {
    logError(
        "Could not connect to evrp-device{} (session handshake failed). "
        "Start `evrp-device`, set --device=HOST:PORT, or rely on UDP discovery "
        "(default when --device is unset; see --discovery_port).",
        device_.empty() ? std::string("") : (" at " + device_));
    return 1;
  }
  optionsView_.insert("device", deviceClient->serverAddress());

  evrp::Ioc ioc;
  ioc.emplace(deviceClient->playback());
  ioc.emplace(deviceClient->inputListener());

  std::unique_ptr<IEnhancedFileSystem> enhancedFs(
      createEnhancedFileSystem(createFileSystem()));
  ioc.emplace(enhancedFs.get());

  if (playback_) {
    if (playbackPath_.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(prog_.c_str());
      return 1;
    }
    return Playback(std::move(options_), ioc).run();
  }

  return Record(std::move(options_), ioc).run();
}
