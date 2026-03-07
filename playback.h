#pragma once

#include <string>

#include "argparser.h"
#include "asynclogwriter.h"
#include "filesystem.h"
#include "inputeventwriter.h"

class Playback {
 public:
  explicit Playback(const run_options &options);
  int run();

 private:
  run_options options_;
  FileSystem fs_;
  InputEventWriter event_writer_;
  AsyncLogWriter log_writer_;
};
