#pragma once

#include <string>

#include "argparser.h"
#include "evrp/sdk/ioc.h"
#include "filesystem.h"
#include "inputeventwriter.h"

class Playback {
 public:
  explicit Playback(const RunOptions &options);
  int run();
  /// Plays via evrp-device: event files (streaming), embedded Lua lines, or .lua
  /// scripts (Lua runs locally; injection uses gRPC). Ioc must hold IPlayback.
  int runWithIoc(const evrp::Ioc &ioc);

 private:
  RunOptions options_;
  FileSystem fs_;
  InputEventWriter eventWriter_;
};
