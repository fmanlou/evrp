#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "input_device.h"

static void print_usage(const char* prog) {
  std::cout << "Usage: " << prog << " -r [touchpad] [mouse] [keyboard] ...\n"
            << "  -r: start recording. With no types, record touchpad, mouse, "
               "keyboard.\n";
}

static bool parse_kind(const std::string& s, std::string* out_label) {
  if (s == "touchpad" || s == "mouse" || s == "keyboard") {
    *out_label = s;
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || std::string(argv[1]) != "-r") {
    print_usage(argv[0]);
    return 1;
  }

  std::vector<std::string> kinds;
  for (int i = 2; i < argc; ++i) {
    std::string label;
    if (!parse_kind(argv[i], &label)) {
      std::cout << "Unknown device type: " << argv[i] << std::endl;
      print_usage(argv[0]);
      return 1;
    }
    kinds.push_back(label);
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

  record_events_multi(targets);

  for (const auto& t : targets) close(t.fd);
  return 0;
}
