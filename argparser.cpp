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
            << "  -q: quiet, no console output.\n";
}

bool parse_kind(const std::string& s, std::string* out_label) {
  if (s == "touchpad" || s == "mouse" || s == "keyboard") {
    *out_label = s;
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

        std::string label;
        if (!parse_kind(argv[i], &label)) break;
        options.kinds.push_back(label);
        ++i;
      }
      break;
    }
  }

  if (options.recording && options.kinds.empty()) {
    options.kinds = {"touchpad", "mouse", "keyboard"};
  }
  return options;
}

