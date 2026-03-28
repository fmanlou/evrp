#pragma once

#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"

namespace evrp::device::server {

// 进程内录制缓存与回放占位：`upload` 写入事件序列；`playback` 在无缓存时失败；
// 事件注入（uinput 等）未实现时仍返回成功及业务 code 0。
class LocalPlayback final : public api::IPlayback {
 public:
  LocalPlayback() = default;

  LocalPlayback(const LocalPlayback&) = delete;
  LocalPlayback& operator=(const LocalPlayback&) = delete;

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* result_out) override;

  bool playback(api::OperationResult* result_out) override;

  bool stop_playback() override;

 private:
  mutable std::mutex mu_;
  std::vector<api::InputEvent> cached_;
  bool playing_{false};
};

}  // namespace evrp::device::server
