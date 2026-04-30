#pragma once

#include "evrp/sdk/stringkeystore.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IInputListener;
}

class Record {
 public:
  Record(MapStringKeyStoreCore parsed, evrp::device::api::IInputListener *listener,
         IEnhancedFileSystem *fs);

  Record(MapStringKeyStoreCore parsed, const evrp::Ioc &ioc);

  int run();

 private:
  MapStringKeyStoreCore parsed_;
  StringKeyStore parsedView_;
  evrp::device::api::IInputListener *listener_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
