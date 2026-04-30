#pragma once

#include "argparser.h"
#include "evrp/sdk/ioc.h"

class IEnhancedFileSystem;

namespace evrp::device::api {
class IPlayback;
}

class Playback {
 public:
  Playback(const std::map<std::string, std::any>& parsed,
           evrp::device::api::IPlayback *playback, IEnhancedFileSystem *fs);

  Playback(const std::map<std::string, std::any>& parsed, const evrp::Ioc &ioc);

  int run();

 private:
  std::map<std::string, std::any> parsed_;
  evrp::device::api::IPlayback *remote_{nullptr};
  IEnhancedFileSystem *fs_{nullptr};
};
