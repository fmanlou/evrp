#pragma once

#include <string>

#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

class Runner {
 public:
  explicit Runner(MemorySetting settings);

  int run();

 private:
  MemorySetting settings_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string device_;
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
