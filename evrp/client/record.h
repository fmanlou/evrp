#pragma once

#include "argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(const std::map<std::string, std::any>& parsed,
         evrp::device::api::IInputListener *listener, IEnhancedFileSystem *fs);

  Record(const std::map<std::string, std::any>& parsed, const evrp::Ioc &ioc);

  int run();

 private:
  std::map<std::string, std::any> parsed_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
