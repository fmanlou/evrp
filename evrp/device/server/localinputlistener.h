#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "filesystem.h"

namespace evrp::device::server {

// 进程内输入监听：对外接口将工作投递至 `dispatch_worker_` 单线程顺序执行（含阻塞 poll），
// 多线程调用方仅阻塞在各自 `future` 上。`mu_` 保护 devices_ 等（仅 worker 使用，避免与其它
// 逻辑交错）。需 root 或 input 组权限。
class LocalInputListener final : public api::IInputListener {
 public:
  LocalInputListener();

  // 入队收尾并 join worker；之后 post_sync 返回默认值。可重复调用。
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

  template <typename R>
  R post_sync(std::function<R()> fn);

  void post_void(std::function<void()> fn);

  void dispatch_loop();

  void close_devices();

  bool start_listening_on_worker(const std::vector<api::DeviceKind>& kinds);
  std::vector<api::InputEvent> read_input_events_on_worker();
  bool wait_for_input_event_on_worker(int timeout_ms);
  void cancel_listening_on_worker();

  FileSystem fs_;
  std::mutex mu_;
  std::atomic<bool> listening_active_{false};
  std::atomic<bool> disposed_{false};

  std::mutex dispatch_mu_;
  std::condition_variable dispatch_cv_;
  std::queue<std::function<void()>> dispatch_q_;
  bool dispatch_stop_{false};
  std::thread dispatch_worker_;

  std::vector<TrackedDevice> devices_;
  std::set<size_t> poll_ready_indices_;
};

template <typename R>
R LocalInputListener::post_sync(std::function<R()> fn) {
  if (disposed_.load(std::memory_order_acquire)) {
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
