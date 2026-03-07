#pragma once

#include "argparser.h"

#include <ostream>
#include <string>
#include <vector>

struct RecordTarget {
  int fd;
  std::string label;
  std::string path;
};

class FileSystem;

std::vector<RecordTarget> collect_targets(FileSystem* fs,
                                          const std::vector<std::string>& kinds);
void close_targets(FileSystem* fs, const std::vector<RecordTarget>& targets);
void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out,
                         bool log_events_to_console = true);

int run_recording(const run_options& options);
