#pragma once

#include <string>

#include "evrp/client/argparser.h"
#include "evrp/sdk/logger.h"

class Runner {
 public:
  explicit Runner(ParsedOptions options);

  int run();

 private:
  ParsedOptions options_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string device_;
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
