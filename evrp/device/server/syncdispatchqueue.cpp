#include "evrp/device/server/syncdispatchqueue.h"

#include <memory>

namespace evrp::device::server {

SyncDispatchQueue::SyncDispatchQueue()
    : worker_([this]() { dispatchLoop(); }) {}

void SyncDispatchQueue::postVoid(std::function<void()> fn) {
  if (shutdownDone_.load(std::memory_order_acquire)) {
    return;
  }
  auto task = std::make_shared<std::packaged_task<void()>>(std::move(fn));
  std::future<void> fut = task->get_future();
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (stop_) {
      return;
    }
    q_.push([task]() { (*task)(); });
  }
  cv_.notify_one();
  fut.get();
}

void SyncDispatchQueue::dispatchLoop() {
  for (;;) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lock(mu_);
      cv_.wait(lock, [this]() { return stop_ || !q_.empty(); });
      if (stop_ && q_.empty()) {
        return;
      }
      job = std::move(q_.front());
      q_.pop();
    }
    job();
  }
}

void SyncDispatchQueue::shutdown(std::function<void()> final_task) {
  std::future<void> cleanup_done;
  const bool has_final = static_cast<bool>(final_task);
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (stop_) {
      return;
    }
    if (has_final) {
      auto pt =
          std::make_shared<std::packaged_task<void()>>(std::move(final_task));
      cleanup_done = pt->get_future();
      q_.push([pt]() { (*pt)(); });
    }
    stop_ = true;
  }
  cv_.notify_all();
  if (has_final) {
    cleanup_done.wait();
  }
  if (worker_.joinable()) {
    worker_.join();
  }
  shutdownDone_.store(true, std::memory_order_release);
}

SyncDispatchQueue::~SyncDispatchQueue() { shutdown(); }

}  // namespace evrp::device::server
