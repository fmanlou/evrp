#pragma once

#include <string>

#include "argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(const RunOptions &options, evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(const RunOptions &options, const evrp::Ioc &ioc);

  int run();

 private:
  RunOptions options_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
