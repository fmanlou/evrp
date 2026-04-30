#pragma once

#include "evrp/sdk/stringkeystore.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(MapStringKeyStore parsed, evrp::device::api::IPlayback *playback,
           IEnhancedFileSystem *fs);

  Playback(MapStringKeyStore parsed, const evrp::Ioc &ioc);

  int run();

 private:
  MapStringKeyStore parsed_;
  StringKeyStore parsedView_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
