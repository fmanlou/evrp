#include "argparser.h"
#include "cursor/cursorpos.h"
#include "logger.h"
#include "lua/luabindings.h"
#include "playback.h"
#include "record.h"

int main(int argc, char *argv[]) {
  Logger logger("evrp");
  gLogger = &logger;

  CursorPos cursor;
  gCursor = &cursor;

  RunOptions options = parseOptions(argc, argv);
  gLogger->setLevel(options.logLevel);

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

  if (options.playback) {
    if (options.playbackPath.empty()) {
      logError("Playback (--playback) requires a file path.");
      printUsage(argv[0]);
      return 1;
    }
    return Playback(options).run();
  }

  return Record(options).run();
}
