#include "argparser.h"

#include <iostream>
#include <string>

void print_usage(const char *prog) {
  std::cout
      << "Usage: " << prog
      << " -r [-o FILE] [--log-level=LEVEL] [touchpad] [mouse] [keyboard] ...\n"
      << "       " << prog << " -p FILE [--log-level=LEVEL]\n"
      << "       " << prog << " -l FILE [--log-level=LEVEL]\n"
      << "  -r: start recording. With no types, record touchpad, mouse, "
         "keyboard.\n"
      << "  -p FILE: playback events from FILE into input subsystem.\n"
      << "  -l FILE: execute Lua script.\n"
      << "  -o FILE: write recording to FILE (default: stdout).\n"
      << "  --log-level=LEVEL: error|warn|info|debug|trace (default: info).\n";
}

bool parse_kind(const std::string &s, DeviceId *out_id) {
  DeviceId id = device_id_from_label(s);
  if (id != DeviceId::Unknown) {
    *out_id = id;
    return true;
  }
  return false;
}

static bool parse_log_level(const std::string &arg, LogLevel *out) {
  const std::string prefix = "--log-level=";
  if (arg.size() < prefix.size() || arg.substr(0, prefix.size()) != prefix) {
    return false;
  }
  *out = log_level_from_string(arg.substr(prefix.size()));
  return true;
}

run_options parse_options(int argc, char *argv[]) {
  run_options options;
  options.recording = false;
  options.playback = false;
  options.lua_script = false;
  options.log_level = LogLevel::Info;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") {
      if (i + 1 < argc) options.output_path = argv[++i];
    } else if (parse_log_level(argv[i], &options.log_level)) {
      // parsed
    } else if (std::string(argv[i]) == "-p") {
      options.playback = true;
      if (i + 1 < argc) options.playback_path = argv[++i];
    } else if (std::string(argv[i]) == "-l") {
      options.lua_script = true;
      if (i + 1 < argc) options.lua_script_path = argv[++i];
    } else if (std::string(argv[i]) == "-r") {
      options.recording = true;
      ++i;
      while (i < argc) {
        if (std::string(argv[i]) == "-o") {
          if (i + 1 < argc) options.output_path = argv[++i];
          ++i;
          continue;
        }
        if (parse_log_level(argv[i], &options.log_level)) {
          ++i;
          continue;
        }
        if (std::string(argv[i]) == "-p") {
          options.playback = true;
          if (i + 1 < argc) options.playback_path = argv[++i];
          ++i;
          continue;
        }
        if (std::string(argv[i]) == "-l") {
          options.lua_script = true;
          if (i + 1 < argc) options.lua_script_path = argv[++i];
          ++i;
          continue;
        }

        DeviceId id;
        if (!parse_kind(argv[i], &id)) break;
        options.kinds.push_back(id);
        ++i;
      }
      break;
    }
  }

  if (options.recording && options.kinds.empty()) {
    options.kinds = {DeviceId::Touchpad, DeviceId::Mouse, DeviceId::Keyboard};
  }
  return options;
}
