#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "evrp/sdk/types.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/keyboard/keyboarddevice.h"
#include "evrp/sdk/logger.h"

class IEnhancedFileSystem;
class ISetting;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(std::shared_ptr<ISetting> setting,
         evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(std::shared_ptr<ISetting> setting, const evrp::Ioc &ioc);

  void setExternalCancelFlag(std::atomic<bool>* flag) { externalCancel_ = flag; }

  int run();

 private:
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
  logging::LogLevel logLevel_{};
  std::vector<evrp::sdk::DeviceKind> kinds_;
  std::string device_;
  std::string outputPath_;
  KeyboardCtrlCFilterMode keyboardCtrlCFilterMode_{
      KeyboardCtrlCFilterMode::kEndingOnly};
  keyboard_filter_state keyboardFilterState_{};
  std::atomic<bool>* externalCancel_{nullptr};
};
