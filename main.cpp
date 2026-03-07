#include <iostream>
#include <string>
#include <vector>

#include "argparser.h"
#include "filesystem/filesystem.h"
#include "playback.h"
#include "record.h"

int main(int argc, char* argv[]) {
  FileSystem fs;
  run_options options = parse_options(argc, argv);

  if (options.recording && options.playback) {
    std::cerr << "Cannot use -r and -p at the same time." << std::endl;
    print_usage(argv[0]);
    return 1;
  }

  if (!options.recording && !options.playback) {
    print_usage(argv[0]);
    return 1;
  }

  if (options.playback) {
    if (options.playback_path.empty()) {
      std::cerr << "Playback mode requires a file path after -p." << std::endl;
      print_usage(argv[0]);
      return 1;
    }
    return playback_file_to_uinput(options.playback_path, options.quiet);
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

  record_events_multi(targets, fs.output_stream(),
                     options.quiet ? nullptr : &std::cout);
  close_targets(&fs, targets);
  return 0;
}
