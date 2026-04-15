#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "logger.h"

struct run_options {
  bool recording;
  bool playback;
  LogLevel log_level;
  std::string playback_path;
  std::string output_path;
  std::vector<evrp::device::api::DeviceKind> kinds;
  // Playback only: whether to execute [leading]/[trailing] waits (default true).
  bool execute_wait_before_first;
  bool execute_wait_after_last;
};

void printUsage(const char *prog);
bool parseKind(const std::string &s, evrp::device::api::DeviceKind *out_kind);
run_options parseOptions(int argc, char *argv[]);
