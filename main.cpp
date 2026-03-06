#include <iostream>
#include <string>
#include <vector>

#include "argparser.h"
#include "filesystem/filesystem.h"
#include "inputdevice.h"

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
