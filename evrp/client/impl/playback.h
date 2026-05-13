#pragma once

#include <string>

#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

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
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
  logging::LogLevel logLevel_{};
  std::string playbackPath_;
  std::string device_;
};
