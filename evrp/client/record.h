#pragma once

#include "evrp/sdk/stringkeystore.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(MapStringKeyStore parsed, evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(MapStringKeyStore parsed, const evrp::Ioc &ioc);

  int run();

 private:
  MapStringKeyStore parsed_;
  StringKeyStore parsedView_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
