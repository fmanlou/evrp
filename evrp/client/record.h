#pragma once

#include <string>

#include "argparser.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"

class Record {
 public:
  Record(const RunOptions &options, const evrp::Ioc &ioc);
  int run();

 private:
  RunOptions options_;
  const evrp::Ioc &ioc_;
  EnhancedFileSystem fs_;
};
