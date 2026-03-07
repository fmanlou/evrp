#pragma once

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
                         std::ostream* console_out = nullptr);
