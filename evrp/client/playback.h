#pragma once

#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(MemorySetting setting, evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(MemorySetting setting, const evrp::Ioc &ioc);

  int run();

 private:
  MemorySetting setting_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
