#include <iostream>
#include <string>
#include <vector>

#include "filesystem/filesystem.h"
#include "inputdevice.h"

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
  FileSystem fs;
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

    int fd = fs.open_read_only(path.c_str(), false);
    if (fd < 0) {
      fs.print_error(path.c_str());
      continue;
    }
    targets.push_back({fd, kind, path});
  }

  if (targets.empty()) {
    std::cout << "No devices to record." << std::endl;
    return 1;
  }

  if (!fs.open_output(output_path)) {
    std::cerr << fs.error_message() << std::endl;
    for (const auto& t : targets) fs.close_fd(t.fd);
    return 1;
  }
  record_events_multi(targets, fs.output_stream());

  for (const auto& t : targets) fs.close_fd(t.fd);
  return 0;
}
