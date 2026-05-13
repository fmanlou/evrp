#pragma once

#include <memory>
#include <string>

#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/isetting.h"
#include "evrp/sdk/setting/memorysetting.h"

struct RunnerSetting {
  explicit RunnerSetting(const ISetting& settings);

  std::string program;
  bool recording{false};
  bool playback{false};
  std::string playbackPath;
  logging::LogLevel logLevel{logging::LogLevel::Info};
};

class Runner {
 public:
  explicit Runner(std::shared_ptr<MemorySetting> settings);

  int run();

 private:
  std::shared_ptr<MemorySetting> settings_;
  RunnerSetting runnerSetting_;
};
