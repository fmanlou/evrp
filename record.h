#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "argparser.h"
#include "evrp/device/api/types.h"
#include "filesystem.h"

struct RecordTarget {
  int fd;
  evrp::device::api::DeviceKind kind;
  std::string path;
};

class Record {
 public:
  explicit Record(const run_options &options);
  int run();

 private:
  std::vector<RecordTarget> collectTargets();
  void closeTargets();
  void recordEvents();

  run_options options_;
  FileSystem fs_;
  std::vector<RecordTarget> targets_;
};
