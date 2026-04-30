#pragma once

#include "evrp/sdk/stringkeystore.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(MapStringKeyStoreCore parsed, evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(MapStringKeyStoreCore parsed, const evrp::Ioc &ioc);

  int run();

 private:
  MapStringKeyStoreCore parsed_;
  StringKeyStore parsedView_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
