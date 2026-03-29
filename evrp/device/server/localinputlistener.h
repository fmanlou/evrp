#pragma once

#include <atomic>
#include <mutex>
#include <set>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "filesystem.h"

namespace evrp::device::server {

// 进程内输入监听：同步实现（单线程使用或外包给 DispatchedInputListener）。
// start_listening 打开 evdev；wait_for_input_event 阻塞 poll；read_input_events
// 消费 poll 就绪缓存。需 root 或 input 组权限。
class LocalInputListener final : public api::IInputListener {
 public:
  LocalInputListener() = default;

  // 调用 cancel_listening() 后置 disposed_；之后接口无操作；析构时也会调用；可重复调用。
  void dispose();

  ~LocalInputListener() override;

  LocalInputListener(const LocalInputListener&) = delete;
  LocalInputListener& operator=(const LocalInputListener&) = delete;

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> read_input_events() override;

  bool wait_for_input_event(int timeout_ms) override;

  void cancel_listening() override;

  bool is_listening() const override;

 private:
  struct TrackedDevice {
    int fd{-1};
    api::DeviceKind kind{api::DeviceKind::kUnspecified};
  };

  void close_devices();

  FileSystem fs_;
  std::mutex mu_;
  std::atomic<bool> listening_active_{false};
  std::atomic<bool> disposed_{false};
  std::vector<TrackedDevice> devices_;
  std::set<size_t> poll_ready_indices_;
};

}  // namespace evrp::device::server
