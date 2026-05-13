#pragma once

#include <memory>
#include <string>

#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"

class IEnhancedFileSystem;
class ISetting;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(std::shared_ptr<ISetting> setting,
           evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(std::shared_ptr<ISetting> setting, const evrp::Ioc &ioc);

  int run();

 private:
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
  logging::LogLevel logLevel_{};
  std::string playbackPath_;
  std::string device_;
};
