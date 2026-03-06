#include "argparser.h"

#include <iostream>
#include <string>

void print_usage(const char* prog) {
  std::cout << "Usage: " << prog
            << " -r [-o FILE] [touchpad] [mouse] [keyboard] ...\n"
            << "  -r: start recording. With no types, record touchpad, mouse, "
               "keyboard.\n"
            << "  -o FILE: write events to FILE (default: stdout).\n";
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

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") {
      if (i + 1 < argc) options.output_path = argv[++i];
    } else if (std::string(argv[i]) == "-r") {
      options.recording = true;
      ++i;
      while (i < argc) {
        if (std::string(argv[i]) == "-o") {
          if (i + 1 < argc) options.output_path = argv[++i];
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

  if (options.kinds.empty()) options.kinds = {"touchpad", "mouse", "keyboard"};
  return options;
}

