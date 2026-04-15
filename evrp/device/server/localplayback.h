#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"

namespace evrp::device::server {

// 进程内：`upload` 缓存事件序列；`playback` 通过 `InputEventWriter` 按录制时间间隔写回 evdev。
class LocalPlayback final : public api::IPlayback {
 public:
  LocalPlayback() = default;

  LocalPlayback(const LocalPlayback&) = delete;
  LocalPlayback& operator=(const LocalPlayback&) = delete;

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* result_out) override;

  bool playback(api::OperationResult* result_out,
                evrp::CountingSemaphore* progress_notify = nullptr) override;

  int playbackIndex() const override;

  bool stopPlayback() override;

 private:
  mutable std::mutex mu_;
  std::vector<api::InputEvent> cached_;
  bool playing_{false};
  std::atomic<bool> stopRequested_{false};
  int currentEventIndex_{-1};
};

}  // namespace evrp::device::server
