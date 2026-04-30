#pragma once

#include <memory>
#include <string>

#include "argparser.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"

class Playback {
 public:
  Playback(const RunOptions &options, const evrp::Ioc &ioc);
  int run();

 private:
  RunOptions options_;
  const evrp::Ioc &ioc_;
  std::unique_ptr<IEnhancedFileSystem> fs_;
};
