#pragma once

#include <vector>

#include "evrp/countingsemaphore.h"
#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IPlayback {
 public:
  virtual ~IPlayback() = default;

  virtual bool upload(const std::vector<InputEvent>& events,
                      OperationResult* result_out) = 0;

  virtual bool playback(OperationResult* result_out,
                        evrp::CountingSemaphore* progress_notify = nullptr) = 0;

  virtual int playback_index() const = 0;

  virtual bool stop_playback() = 0;
};

}
