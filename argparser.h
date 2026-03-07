#pragma once

#include <string>
#include <vector>

struct run_options {
  bool recording;
  bool playback;
  bool quiet;
  std::string playback_path;
  std::string output_path;
  std::vector<std::string> kinds;
};

void print_usage(const char* prog);
bool parse_kind(const std::string& s, std::string* out_label);
run_options parse_options(int argc, char* argv[]);

