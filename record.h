#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "argparser.h"
#include "deviceid.h"
#include "filesystem.h"

struct RecordTarget {
  int fd;
  DeviceId id;
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
