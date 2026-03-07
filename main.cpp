#include "argparser.h"
#include "playback.h"
#include "record.h"

#include <iostream>

int main(int argc, char* argv[]) {
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
      return 1;
    }
    return Playback(options.playback_path, options.quiet).run();
  }

  return Record(options).run();
}
