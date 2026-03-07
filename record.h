#pragma once

#include "argparser.h"
#include "asynclogwriter.h"
#include "filesystem/filesystem.h"

#include <ostream>
#include <string>
#include <vector>

struct RecordTarget {
  int fd;
  std::string label;
  std::string path;
};

class Record {
 public:
  explicit Record(const run_options& options);
  int run();

 private:
  std::vector<RecordTarget> collect_targets();
  void close_targets();
  void record_events();

  run_options options_;
  FileSystem fs_;
  std::vector<RecordTarget> targets_;
};
