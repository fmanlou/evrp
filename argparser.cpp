#include "argparser.h"

#include <iostream>
#include <string>

void print_usage(const char* prog) {
  std::cout << "Usage: " << prog
            << " -r [-o FILE] [-q] [touchpad] [mouse] [keyboard] ...\n"
            << "       " << prog << " -p FILE [-q]\n"
            << "  -r: start recording. With no types, record touchpad, mouse, "
               "keyboard.\n"
            << "  -p FILE: playback events from FILE into input subsystem.\n"
            << "  -o FILE: write recording to FILE (default: stdout).\n"
            << "  -q: quiet, suppress event log only (status/errors still shown).\n";
}

bool parse_kind(const std::string& s, DeviceId* out_id) {
  DeviceId id = device_id_from_label(s);
  if (id != DeviceId::Unknown) {
    *out_id = id;
    return true;
  }
  return false;
}

run_options parse_options(int argc, char* argv[]) {
  run_options options;
  options.recording = false;
  options.playback = false;
  options.quiet = false;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") {
      if (i + 1 < argc) options.output_path = argv[++i];
    } else if (std::string(argv[i]) == "-q") {
      options.quiet = true;
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
        if (std::string(argv[i]) == "-q") {
          options.quiet = true;
          ++i;
          continue;
        }
        if (std::string(argv[i]) == "-p") {
          options.playback = true;
          if (i + 1 < argc) options.playback_path = argv[++i];
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

