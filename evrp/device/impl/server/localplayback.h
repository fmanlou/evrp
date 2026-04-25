#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"

namespace evrp::device::server {

class LocalPlayback final : public api::IPlayback {
 public:
  LocalPlayback() = default;

  LocalPlayback(const LocalPlayback&) = delete;
  LocalPlayback& operator=(const LocalPlayback&) = delete;

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* resultOut) override;

  bool playback(api::OperationResult* resultOut,
                evrp::CountingSemaphore* progressNotify = nullptr) override;

  int playbackIndex() const override;

  bool stopPlayback() override;

 private:
  mutable std::mutex mu_;
  std::vector<api::InputEvent> cached_;
  bool playing_{false};
  std::atomic<bool> stopRequested_{false};
  int currentEventIndex_{-1};
};

}
