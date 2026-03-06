#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "input_device.h"

static void print_usage(const char* prog) {
  std::cout << "Usage: " << prog
            << " -r [-o FILE] [touchpad] [mouse] [keyboard] ...\n"
            << "  -r: start recording. With no types, record touchpad, mouse, "
               "keyboard.\n"
            << "  -o FILE: write events to FILE (default: stdout).\n";
}

static bool parse_kind(const std::string& s, std::string* out_label) {
  if (s == "touchpad" || s == "mouse" || s == "keyboard") {
    *out_label = s;
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {
  std::string output_path;
  bool recording = false;
  std::vector<std::string> kinds;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") {
      if (i + 1 < argc) output_path = argv[++i];
    } else if (std::string(argv[i]) == "-r") {
      recording = true;
      ++i;
      while (i < argc) {
        if (std::string(argv[i]) == "-o") {
          if (i + 1 < argc) output_path = argv[++i];
          ++i;
          continue;
        }
        std::string label;
        if (parse_kind(argv[i], &label)) {
          kinds.push_back(label);
          ++i;
        } else {
          break;
        }
      }
      break;
    }
  }

  if (!recording) {
    print_usage(argv[0]);
    return 1;
  }
  if (kinds.empty()) kinds = {"touchpad", "mouse", "keyboard"};

  std::vector<RecordTarget> targets;
  for (const auto& kind : kinds) {
    std::string path;
    if (kind == "touchpad")
      path = find_first_touchpad();
    else if (kind == "mouse")
      path = find_first_mouse();
    else
      path = find_first_keyboard();

    if (path.empty()) {
      std::cout << "No " << kind << " detected. Try running with sudo."
                << std::endl;
      continue;
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
      std::perror(path.c_str());
      continue;
    }
    targets.push_back({fd, kind, path});
  }

  if (targets.empty()) {
    std::cout << "No devices to record." << std::endl;
    return 1;
  }

  if (!output_path.empty()) {
    std::ofstream out(output_path);
    if (!out) {
      std::perror(output_path.c_str());
      for (const auto& t : targets) close(t.fd);
      return 1;
    }
    record_events_multi(targets, out);
  } else {
    record_events_multi(targets, std::cout);
  }

  for (const auto& t : targets) close(t.fd);
  return 0;
}
