#include "evrp/device/server/dispatchedinputlistener.h"

#include <memory>

namespace evrp::device::server {

DispatchedInputListener::DispatchedInputListener(api::IInputListener& inner)
    : inner_(inner), dispatch_worker_([this]() { dispatch_loop(); }) {}

void DispatchedInputListener::post_void(std::function<void()> fn) {
  if (shutdown_done_.load(std::memory_order_acquire)) {
    return;
  }
  auto task = std::make_shared<std::packaged_task<void()>>(std::move(fn));
  std::future<void> fut = task->get_future();
  {
    std::lock_guard<std::mutex> lock(dispatch_mu_);
    if (dispatch_stop_) {
      return;
    }
    dispatch_q_.push([task]() { (*task)(); });
  }
  dispatch_cv_.notify_one();
  fut.get();
}

void DispatchedInputListener::dispatch_loop() {
  for (;;) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lock(dispatch_mu_);
      dispatch_cv_.wait(lock, [this]() {
        return dispatch_stop_ || !dispatch_q_.empty();
      });
      if (dispatch_stop_ && dispatch_q_.empty()) {
        return;
      }
      job = std::move(dispatch_q_.front());
      dispatch_q_.pop();
    }
    job();
  }
}

void DispatchedInputListener::shutdown() {
  std::future<void> cleanup_done;
  {
    std::lock_guard<std::mutex> lock(dispatch_mu_);
    if (dispatch_stop_) {
      return;
    }
    auto pt = std::make_shared<std::packaged_task<void()>>([this]() {
      inner_.cancel_listening();
    });
    cleanup_done = pt->get_future();
    dispatch_q_.push([pt]() { (*pt)(); });
    dispatch_stop_ = true;
  }
  dispatch_cv_.notify_all();
  cleanup_done.wait();
  if (dispatch_worker_.joinable()) {
    dispatch_worker_.join();
  }
  shutdown_done_.store(true, std::memory_order_release);
}

DispatchedInputListener::~DispatchedInputListener() { shutdown(); }

bool DispatchedInputListener::start_listening(
    const std::vector<api::DeviceKind>& kinds) {
  return post_sync<bool>(
      [this, kinds]() { return inner_.start_listening(kinds); });
}

std::vector<api::InputEvent> DispatchedInputListener::read_input_events() {
  return post_sync<std::vector<api::InputEvent>>(
      [this]() { return inner_.read_input_events(); });
}

bool DispatchedInputListener::wait_for_input_event(int timeout_ms) {
  return post_sync<bool>([this, timeout_ms]() {
    return inner_.wait_for_input_event(timeout_ms);
  });
}

void DispatchedInputListener::cancel_listening() {
  post_void([this]() { inner_.cancel_listening(); });
}

bool DispatchedInputListener::is_listening() const {
  return inner_.is_listening();
}

}  // namespace evrp::device::server
