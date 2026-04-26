#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "logger.h"

struct RunOptions {
  bool recording;
  bool playback;
  logging::LogLevel logLevel;
  std::string playbackPath;
  std::string outputPath;
  /// gRPC target for evrp-device (host:port).
  std::string device;
  std::vector<evrp::device::api::DeviceKind> kinds;
  
  bool executeWaitBeforeFirst;
  bool executeWaitAfterLast;
};

void printUsage(const char *prog);
bool parseKind(const std::string &s, evrp::device::api::DeviceKind *outKind);
RunOptions parseOptions(int argc, char *argv[]);
