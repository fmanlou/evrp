#include "argparser.h"

#include <iostream>
#include <string>

void print_usage(const char *prog) {
  std::cout
      << "Usage: " << prog
      << " -r [-o FILE] [--log-level=LEVEL] [touchpad] [touchscreen] [mouse] "
         "[keyboard] ...\n"
      << "       " << prog << " -p FILE [--log-level=LEVEL]\n"
      << "  -r: start recording. With no types, record touchpad, touchscreen, "
         "mouse, keyboard.\n"
      << "  -p FILE: playback events or run Lua script (.lua). Non-event lines "
         "in event files are executed as Lua.\n"
      << "  -o FILE: write recording to FILE (default: stdout).\n"
      << "  --log-level=LEVEL: error|warn|info|debug|trace (default: info).\n"
      << "  --wait-leading=yes|no: during playback, execute [leading] wait "
         "(default: yes).\n"
      << "  --wait-trailing=yes|no: during playback, execute [trailing] wait "
         "(default: yes).\n";
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

static bool parse_yes_no(const std::string &arg, const std::string &prefix,
                        bool *out) {
  if (arg.size() < prefix.size() || arg.substr(0, prefix.size()) != prefix) {
    return false;
  }
  std::string val = arg.substr(prefix.size());
  if (val == "yes" || val == "true" || val == "1") {
    *out = true;
    return true;
  }
  if (val == "no" || val == "false" || val == "0") {
    *out = false;
    return true;
  }
  return false;
}

run_options parse_options(int argc, char *argv[]) {
  run_options options;
  options.recording = false;
  options.playback = false;
  options.log_level = LogLevel::Info;
  options.execute_wait_before_first = true;
  options.execute_wait_after_last = true;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") {
      if (i + 1 < argc) options.output_path = argv[++i];
    } else if (parse_log_level(argv[i], &options.log_level)) {
      // parsed
    } else if (parse_yes_no(argv[i], "--wait-leading=",
                            &options.execute_wait_before_first)) {
      // parsed
    } else if (parse_yes_no(argv[i], "--wait-trailing=",
                            &options.execute_wait_after_last)) {
      // parsed
    } else if (std::string(argv[i]) == "-p") {
      options.playback = true;
      if (i + 1 < argc) options.playback_path = argv[++i];
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
        if (parse_yes_no(argv[i], "--wait-leading=",
                        &options.execute_wait_before_first)) {
          ++i;
          continue;
        }
        if (parse_yes_no(argv[i], "--wait-trailing=",
                        &options.execute_wait_after_last)) {
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
    options.kinds = {DeviceId::Touchpad, DeviceId::Touchscreen, DeviceId::Mouse,
                     DeviceId::Keyboard};
  }
  return options;
}
