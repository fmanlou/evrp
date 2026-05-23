#pragma once

#include <memory>
#include <string>

#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/setting/isetting.h"
#include "evrp/sdk/setting/memorysetting.h"

namespace evrp::server {
class Evrp;
}

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
  Runner(std::shared_ptr<MemorySetting> settings, evrp::server::Evrp* evrp);

  int run();

 private:
  std::shared_ptr<MemorySetting> settings_;
  RunnerSetting runnerSetting_;
  evrp::server::Evrp* evrp_;
};
