#pragma once

#include <memory>
#include <string>

#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

class Runner {
 public:
  explicit Runner(std::shared_ptr<MemorySetting> settings);

  int run();

 private:
  std::shared_ptr<MemorySetting> settings_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
