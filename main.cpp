#include "argparser.h"
#include "cursor/cursorpos.h"
#include "logger.h"
#include "lua/lua_bindings.h"
#include "playback.h"
#include "record.h"

int main(int argc, char *argv[]) {
  Logger logger;
  g_logger = &logger;

  CursorPos cursor;
  g_cursor = &cursor;

  run_options options = parse_options(argc, argv);
  g_logger->set_level(options.log_level);

  int mode_count = (options.recording ? 1 : 0) + (options.playback ? 1 : 0);
  if (mode_count > 1) {
    log_error("Cannot use -r and -p at the same time.");
    print_usage(argv[0]);
    return 1;
  }

  if (mode_count == 0) {
    print_usage(argv[0]);
    return 1;
  }

  if (options.playback) {
    if (options.playback_path.empty()) {
      log_error("Playback (-p) requires a file path.");
      print_usage(argv[0]);
      return 1;
    }
    return Playback(options).run();
  }

  return Record(options).run();
}
