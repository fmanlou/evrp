#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "filesystem.h"

namespace evrp::device {

// 进程内输入监听（非 gRPC）：start_listening 打开 evdev 设备；read_input_events 非阻塞
// 轮询一次（timeout 0），有非 EV_SYN 事件则返回，否则返回空 vector；需 root 或 input 组权限。
// listening_active_ 为 std::atomic<bool>；cancel_listening 先置 false 再加锁关设备；dispose 与析构均委托 cancel_listening；devices_ 仅在 mu_ 下访问。
class LocalInputListener final : public api::IInputListener {
 public:
  LocalInputListener() = default;

  // 结束监听并关闭设备；内部调用 cancel_listening()；析构时也会调用；可重复调用。
  void dispose();

  ~LocalInputListener() override;

  LocalInputListener(const LocalInputListener&) = delete;
  LocalInputListener& operator=(const LocalInputListener&) = delete;

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  // 单次非阻塞读取；当前无可用非 EV_SYN 事件则返回空 vector。
  // 未先 start_listening、已 cancel_listening（会话已结束）或尚无已打开设备时返回空 vector。
  std::vector<api::InputEvent> read_input_events() override;

  void cancel_listening() override;

 private:
  struct TrackedDevice {
    int fd{-1};
    api::DeviceKind kind{api::DeviceKind::kUnspecified};
  };

  void close_devices_unlocked();

  FileSystem fs_;
  std::mutex mu_;
  std::atomic<bool> listening_active_{false};
  std::vector<TrackedDevice> devices_;
};

}  // namespace evrp::device
