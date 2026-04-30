#pragma once

#include "evrp/client/argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(ParsedOptions parsed, evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(ParsedOptions parsed, const evrp::Ioc &ioc);

  int run();

 private:
  ParsedOptions parsed_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
