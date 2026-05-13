#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/sdk/types.h"

namespace evrp::device::server {

class LocalPlayback final : public api::IPlayback {
 public:
  LocalPlayback() = default;

  LocalPlayback(const LocalPlayback&) = delete;
  LocalPlayback& operator=(const LocalPlayback&) = delete;

  bool upload(const std::vector<evrp::sdk::InputEvent>& events,
              evrp::sdk::StatusCode* resultOut) override;

  bool playback(evrp::sdk::StatusCode* resultOut,
                evrp::CountingSemaphore* progressNotify = nullptr) override;

  int playbackIndex() const override;

  bool isPlayback() const override;

  bool stopPlayback() override;

 private:
  mutable std::mutex mu_;
  std::vector<evrp::sdk::InputEvent> cached_;
  std::atomic<bool> playing_{false};
  std::atomic<bool> stopRequested_{false};
  int currentEventIndex_{-1};
};

}
