#pragma once

#include <string>
#include <vector>

#include "deviceid.h"
#include "logger.h"

struct run_options {
  bool recording;
  bool playback;
  LogLevel log_level;
  std::string playback_path;
  std::string output_path;
  std::vector<DeviceId> kinds;
  // Playback only: whether to execute [leading]/[trailing] waits (default true).
  bool execute_wait_before_first;
  bool execute_wait_after_last;
};

void print_usage(const char *prog);
bool parse_kind(const std::string &s, DeviceId *out_id);
run_options parse_options(int argc, char *argv[]);
