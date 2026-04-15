#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "logger.h"

struct RunOptions {
  bool recording;
  bool playback;
  LogLevel logLevel;
  std::string playbackPath;
  std::string outputPath;
  std::vector<evrp::device::api::DeviceKind> kinds;
  // Playback only: whether to execute [leading]/[trailing] waits (default true).
  bool executeWaitBeforeFirst;
  bool executeWaitAfterLast;
};

void printUsage(const char *prog);
bool parseKind(const std::string &s, evrp::device::api::DeviceKind *outKind);
RunOptions parseOptions(int argc, char *argv[]);
