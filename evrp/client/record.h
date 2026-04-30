#pragma once

#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(MemorySetting parsed, evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(MemorySetting parsed, const evrp::Ioc &ioc);

  int run();

 private:
  MemorySetting parsed_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
