#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace evrp::device::server {

class SyncDispatchQueue {
 public:
  SyncDispatchQueue();
  ~SyncDispatchQueue();

  SyncDispatchQueue(const SyncDispatchQueue&) = delete;
  SyncDispatchQueue& operator=(const SyncDispatchQueue&) = delete;

  void shutdown(std::function<void()> finalTask = {});

  template <typename R>
  R postSync(std::function<R()> fn);

  void postVoid(std::function<void()> fn);

 private:
  void dispatchLoop();

  std::mutex mu_;
  std::condition_variable cv_;
  std::queue<std::function<void()>> q_;
  bool stop_{false};
  std::thread worker_;
  std::atomic<bool> shutdownDone_{false};
};

template <typename R>
R SyncDispatchQueue::postSync(std::function<R()> fn) {
  if (shutdownDone_.load(std::memory_order_acquire)) {
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

}
