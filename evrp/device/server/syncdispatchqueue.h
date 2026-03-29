#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace evrp::device::server {

// 单 worker 串行执行队列任务；`post_sync` / `post_void` 在调用线程阻塞直到任务跑完。
// `shutdown(final_task)` 先入队可选收尾任务再停止线程；析构时若仍未 stop 则不加收尾直接 join。
class SyncDispatchQueue {
 public:
  SyncDispatchQueue();
  ~SyncDispatchQueue();

  SyncDispatchQueue(const SyncDispatchQueue&) = delete;
  SyncDispatchQueue& operator=(const SyncDispatchQueue&) = delete;

  // final_task 在 worker 上于 stop 前执行；可重复调用（第二次为空操作）。
  void shutdown(std::function<void()> final_task = {});

  template <typename R>
  R post_sync(std::function<R()> fn);

  void post_void(std::function<void()> fn);

 private:
  void dispatch_loop();

  std::mutex mu_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> q_;
  bool stop_{false};
  std::thread worker_;
  std::atomic<bool> shutdown_done_{false};
};

template <typename R>
R SyncDispatchQueue::post_sync(std::function<R()> fn) {
  if (shutdown_done_.load(std::memory_order_acquire)) {
    return R{};
  }
  auto task = std::make_shared<std::packaged_task<R()>>(std::move(fn));
  std::future<R> fut = task->get_future();
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (stop_) {
      return R{};
    }
    q_.push([task]() { (*task)(); });
  }
  cv_.notify_one();
  return fut.get();
}

}  // namespace evrp::device::server
