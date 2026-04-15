#pragma once

#include <string>

#include "argparser.h"
#include "filesystem.h"
#include "inputeventwriter.h"

class Playback {
 public:
  explicit Playback(const RunOptions &options);
  int run();

 private:
  RunOptions options_;
  FileSystem fs_;
  InputEventWriter eventWriter_;
};
