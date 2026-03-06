#include <iostream>
#include <string>
#include <vector>

#include "filesystem/filesystem.h"
#include "inputdevice.h"

struct run_options {
  bool recording;
  std::string output_path;
  std::vector<std::string> kinds;
};

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

static run_options parse_options(int argc, char* argv[]) {
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

static std::string find_device_path(const std::string& kind) {
  if (kind == "touchpad") return find_first_touchpad();
  if (kind == "mouse") return find_first_mouse();
  return find_first_keyboard();
}

static std::vector<RecordTarget> collect_targets(
    FileSystem* fs, const std::vector<std::string>& kinds) {
  std::vector<RecordTarget> targets;

  for (const auto& kind : kinds) {
    std::string path = find_device_path(kind);
    if (path.empty()) {
      std::cout << "No " << kind << " detected. Try running with sudo."
                << std::endl;
      continue;
    }

    int fd = fs->open_read_only(path.c_str(), false);
    if (fd < 0) {
      fs->print_error(path.c_str());
      continue;
    }
    targets.push_back({fd, kind, path});
  }

  return targets;
}

static void close_targets(FileSystem* fs, const std::vector<RecordTarget>& targets) {
  for (const auto& t : targets) fs->close_fd(t.fd);
}

int main(int argc, char* argv[]) {
  FileSystem fs;
  run_options options = parse_options(argc, argv);

  if (!options.recording) {
    print_usage(argv[0]);
    return 1;
  }
  std::vector<RecordTarget> targets = collect_targets(&fs, options.kinds);

  if (targets.empty()) {
    std::cout << "No devices to record." << std::endl;
    return 1;
  }

  if (!fs.open_output(options.output_path)) {
    std::cerr << fs.error_message() << std::endl;
    close_targets(&fs, targets);
    return 1;
  }

  record_events_multi(targets, fs.output_stream());
  close_targets(&fs, targets);
  return 0;
}
