#include "evrp/device/server/dispatch/syncdispatchqueue.h"

#include <memory>

namespace evrp::device::server {

SyncDispatchQueue::SyncDispatchQueue(asio::io_context& ioContext)
    : ioContext_(ioContext) {}

SyncDispatchQueue::~SyncDispatchQueue() { shutdown(); }

void SyncDispatchQueue::postVoid(std::function<void()> fn) const {
  if (shutdownDone_.load(std::memory_order_acquire)) {
    return;
  }
  auto task = std::make_shared<std::packaged_task<void()>>(std::move(fn));
  std::future<void> fut = task->get_future();
  asio::post(ioContext_, [task]() { (*task)(); });
  fut.get();
}

void SyncDispatchQueue::shutdown(std::function<void()> finalTask) {
  if (shutdownDone_.load(std::memory_order_acquire)) {
    return;
  }
  std::promise<void> barrier;
  std::future<void> done = barrier.get_future();
  asio::post(ioContext_, [this, ft = std::move(finalTask),
                           p = std::move(barrier)]() mutable {
    if (ft) {
      ft();
    }
    shutdownDone_.store(true, std::memory_order_release);
    p.set_value();
  });
  done.wait();
}

}  // namespace evrp::device::server
