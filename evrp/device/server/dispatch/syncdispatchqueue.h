#pragma once

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <atomic>
#include <functional>
#include <future>
#include <memory>

namespace evrp::device::server {

// 在指定 io_context 所关联的线程上串行执行任务（由外部 run io_context）。
class SyncDispatchQueue {
 public:
  explicit SyncDispatchQueue(asio::io_context& ioContext);
  ~SyncDispatchQueue();

  SyncDispatchQueue(const SyncDispatchQueue&) = delete;
  SyncDispatchQueue& operator=(const SyncDispatchQueue&) = delete;

  void shutdown(std::function<void()> finalTask = {});

  template <typename R>
  R postSync(std::function<R()> fn) const;

  void postVoid(std::function<void()> fn) const;

 private:
  asio::io_context& ioContext_;
  mutable std::atomic<bool> shutdownDone_{false};
};

template <typename R>
R SyncDispatchQueue::postSync(std::function<R()> fn) const {
  if (shutdownDone_.load(std::memory_order_acquire)) {
    return R{};
  }
  auto task = std::make_shared<std::packaged_task<R()>>(std::move(fn));
  std::future<R> fut = task->get_future();
  asio::post(ioContext_, [task]() { (*task)(); });
  return fut.get();
}

}  // namespace evrp::device::server
