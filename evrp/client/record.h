#pragma once

#include "evrp/client/argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(ParsedOptions parsed, evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(ParsedOptions parsed, const evrp::Ioc &ioc);

  int run();

 private:
  ParsedOptions parsed_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
