#pragma once

#include <atomic>
#include <vector>

#include "evrp/device/api/inputlistener.h"

namespace evrp::device {

// 进程内输入监听（非 gRPC）：start_listening → read_input_events 取事件（不自动结束）；
// cancel_listening 结束监听，可在 read 前调用。listening_active_ 为 atomic；kinds_ 并发由调用方保证。
class LocalInputListener final : public api::IInputListener {
 public:
  LocalInputListener() = default;

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  // 返回本段监听的事件（当前为占位，后续可接 evdev）；监听仍保持，直至 cancel_listening。
  // 未先 start_listening 时返回空 vector。
  std::vector<api::InputEvent> read_input_events() override;

  void cancel_listening() override;

 private:
  std::atomic<bool> listening_active_{false};
  std::vector<api::DeviceKind> kinds_;
};

}  // namespace evrp::device
