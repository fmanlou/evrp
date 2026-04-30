#pragma once

#include <string>

#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/logger.h"

class Runner {
 public:
  explicit Runner(MemorySetting options);

  int run();

 private:
  MemorySetting options_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string device_;
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
