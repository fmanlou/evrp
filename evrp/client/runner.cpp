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

int Runner::run(int argc, char *argv[]) {
  RunOptions options = parseOptions(argc, argv);
  logService->setLevel(options.logLevel);

  int modeCount = (options.recording ? 1 : 0) + (options.playback ? 1 : 0);
  if (modeCount > 1) {
    logError("Cannot use --record and --playback at the same time.");
    printUsage(argv[0]);
    return 1;
  }

  if (modeCount == 0) {
    printUsage(argv[0]);
    return 1;
  }

  std::unique_ptr<evrp::device::api::IClient> deviceClient =
      evrp::device::api::makeClient(options.device);
  if (!deviceClient) {
    logError(
        "Could not connect to evrp-device at {} (session handshake failed). "
        "Start `evrp-device` or pass --device=HOST:PORT.",
        options.device);
    return 1;
  }

  evrp::Ioc ioc;
  ioc.emplace(deviceClient->playback());
  ioc.emplace(deviceClient->inputListener());

  std::unique_ptr<IEnhancedFileSystem> enhancedFs(
      createEnhancedFileSystem(createFileSystem()));
  ioc.emplace(enhancedFs.get());

  if (options.playback) {
    if (options.playbackPath.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(argv[0]);
      return 1;
    }
    return Playback(options.parsed, ioc).run();
  }

  return Record(options.parsed, ioc).run();
}
