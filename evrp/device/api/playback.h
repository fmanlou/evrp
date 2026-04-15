#pragma once

#include <vector>

#include "evrp/countingsemaphore.h"
#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IPlayback {
 public:
  virtual ~IPlayback() = default;

  virtual bool upload(const std::vector<InputEvent>& events,
                      OperationResult* resultOut) = 0;

  virtual bool playback(OperationResult* resultOut,
                        evrp::CountingSemaphore* progressNotify = nullptr) = 0;

  virtual int playbackIndex() const = 0;

  virtual bool stopPlayback() = 0;
};

}
