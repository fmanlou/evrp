#include <iostream>

#include "argparser.h"
#include "lua_bindings.h"
#include "playback.h"
#include "record.h"

int main(int argc, char *argv[]) {
  run_options options = parse_options(argc, argv);

  int mode_count = (options.recording ? 1 : 0) + (options.playback ? 1 : 0) +
                   (options.lua_script ? 1 : 0);
  if (mode_count > 1) {
    std::cerr << "Cannot use -r, -p, and -l at the same time." << std::endl;
    print_usage(argv[0]);
    return 1;
  }

  if (mode_count == 0) {
    print_usage(argv[0]);
    return 1;
  }

  if (options.lua_script) {
    if (options.lua_script_path.empty()) {
      std::cerr << "Lua mode (-l) requires a script path." << std::endl;
      print_usage(argv[0]);
      return 1;
    }
    int err = evrp::lua::run_script(options.lua_script_path.c_str());
    return (err == LUA_OK) ? 0 : 1;
  }

  if (options.playback) {
    return Playback(options).run();
  }

  return Record(options).run();
}
