#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

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
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
  logging::LogLevel logLevel_{};
  std::vector<evrp::device::api::DeviceKind> kinds_;
  std::string device_;
  std::string outputPath_;
};
