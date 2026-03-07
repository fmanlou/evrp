#pragma once

#include "deviceid.h"

#include <string>
#include <vector>

struct run_options {
  bool recording;
  bool playback;
  bool quiet;
  std::string playback_path;
  std::string output_path;
  std::vector<DeviceId> kinds;
};

void print_usage(const char* prog);
bool parse_kind(const std::string& s, DeviceId* out_id);
run_options parse_options(int argc, char* argv[]);

