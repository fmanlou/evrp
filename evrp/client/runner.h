#pragma once

#include <any>
#include <map>
#include <string>

#include "evrp/sdk/logger.h"

class Runner {
 public:
  explicit Runner(const std::map<std::string, std::any>& parsed);

  int run();

 private:
  std::map<std::string, std::any> parsed_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string device_;
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
