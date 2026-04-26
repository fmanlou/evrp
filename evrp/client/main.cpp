#include "argparser.h"
#include "cursor/cursorpos.h"
#include "evrp/device/api/client.h"
#include "evrp/sdk/ioc.h"

#include <memory>
#include "logger.h"
#include "playback.h"
#include "record.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp");
  logService = &logSvc;

  CursorPos cursor;
  gCursor = &cursor;

  RunOptions options = parseOptions(argc, argv);
  logService->setLevel(options.logLevel);

  int mode_count = (options.recording ? 1 : 0) + (options.playback ? 1 : 0);
  if (mode_count > 1) {
    logError("Cannot use --record and --playback at the same time.");
    printUsage(argv[0]);
    return 1;
  }

  if (mode_count == 0) {
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

  if (options.playback) {
    if (options.playbackPath.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(argv[0]);
      return 1;
    }
    return Playback(options, ioc).run();
  }

  return Record(options, ioc).run();
}
