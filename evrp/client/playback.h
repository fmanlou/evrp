#pragma once

#include <string>

#include "argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(const RunOptions &options, evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(const RunOptions &options, const evrp::Ioc &ioc);

  int run();

 private:
  RunOptions options_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
