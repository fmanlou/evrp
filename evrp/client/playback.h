#pragma once

#include <string>

#include "argparser.h"
#include "filesystem.h"
#include "inputeventwriter.h"

namespace evrp::device::api {
class IClient;
}

class Playback {
 public:
  explicit Playback(const RunOptions &options);
  int run();
  /// Plays via evrp-device: event files (streaming), embedded Lua lines, or .lua
  /// scripts (Lua runs locally; injection uses gRPC).
  int runWithDeviceClient(evrp::device::api::IClient *client);

 private:
  RunOptions options_;
  FileSystem fs_;
  InputEventWriter eventWriter_;
};
