#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "evrp/device/api/inputlistener.h"

namespace evrp::device::server {

// 装饰器：在单 worker 线程上顺序调用被包装的 `IInputListener`（含阻塞的
// `wait_for_input_event`），供 gRPC 等多线程入口安全复用同一 listener 实现。
class DispatchedInputListener final : public api::IInputListener {
 public:
  explicit DispatchedInputListener(api::IInputListener& inner);
  ~DispatchedInputListener() override;

  DispatchedInputListener(const DispatchedInputListener&) = delete;
  DispatchedInputListener& operator=(const DispatchedInputListener&) = delete;

  // 停止 worker（先入队 `inner_.cancel_listening()` 再 join）。可与析构择一；可重复调用。
  void shutdown();

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> read_input_events() override;

  bool wait_for_input_event(int timeout_ms) override;

  void cancel_listening() override;

  bool is_listening() const override;

 private:
  template <typename R>
  R post_sync(std::function<R()> fn);

  void post_void(std::function<void()> fn);

  void dispatch_loop();

  api::IInputListener& inner_;

  std::mutex dispatch_mu_;
  std::condition_variable dispatch_cv_;
  std::queue<std::function<void()>> dispatch_q_;
  bool dispatch_stop_{false};
  std::thread dispatch_worker_;
  std::atomic<bool> shutdown_done_{false};
};

template <typename R>
R DispatchedInputListener::post_sync(std::function<R()> fn) {
  if (shutdown_done_.load(std::memory_order_acquire)) {
    return R{};
  }
  auto task = std::make_shared<std::packaged_task<R()>>(std::move(fn));
  std::future<R> fut = task->get_future();
  {
    std::lock_guard<std::mutex> lock(dispatch_mu_);
    if (dispatch_stop_) {
      return R{};
    }
    dispatch_q_.push([task]() { (*task)(); });
  }
  dispatch_cv_.notify_one();
  return fut.get();
}

}  // namespace evrp::device::server
