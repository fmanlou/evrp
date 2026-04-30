#pragma once

#include <string>

#include "evrp/sdk/stringkeystore.h"
#include "evrp/sdk/logger.h"

class Runner {
 public:
  explicit Runner(MapStringKeyStore options);

  int run();

 private:
  MapStringKeyStore options_;
  StringKeyStore optionsView_;
  std::string prog_;
  bool recording_{false};
  bool playback_{false};
  std::string device_;
  std::string playbackPath_;
  logging::LogLevel logLevel_{logging::LogLevel::Info};
};
