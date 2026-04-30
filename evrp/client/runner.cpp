#include "evrp/client/runner.h"

#include "evrp/client/playback.h"
#include "evrp/client/record.h"
#include "evrp/device/api/client.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/filesystem/filesystem.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"

#include <memory>

Runner::Runner(ParsedOptions options)
    : options_(std::move(options)),
      prog_(options_.stringOr("program", "evrp")),
      recording_(options_.boolOr("recording")),
      playback_(options_.boolOr("playback")),
      device_(options_.stringOr("device")),
      playbackPath_(options_.stringOr("playbackPath")),
      logLevel_(options_.logLevelOr("logLevel")) {}

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
        "Could not connect to evrp-device at {} (session handshake failed). "
        "Start `evrp-device` or pass --device=HOST:PORT.",
        device_);
    return 1;
  }

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
