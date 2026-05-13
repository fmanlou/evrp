#pragma once

#include <vector>

#include "evrp/sdk/countingsemaphore.h"
#include "evrp/sdk/types.h"

namespace evrp::device::api {

class IPlayback {
 public:
  virtual ~IPlayback() = default;

  virtual bool upload(const std::vector<InputEvent>& events,
                      StatusCode* resultOut) = 0;

  virtual bool playback(StatusCode* resultOut,
                        evrp::CountingSemaphore* progressNotify = nullptr) = 0;

  virtual int playbackIndex() const = 0;

  virtual bool isPlayback() const = 0;

  virtual bool stopPlayback() = 0;
};

}
