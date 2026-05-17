#include "evrp/sdk/log/asynclogbuffer.h"

namespace evrp::sdk {

AsyncLogBuffer::~AsyncLogBuffer() {
  std::lock_guard<std::mutex> lock(mutex_);
  closed_ = true;
  ready_.notify_all();
}

void AsyncLogBuffer::push(logging::LogLevel level, const std::string& line) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (closed_) {
    return;
  }
  while (queue_.size() >= kMaxQueue) {
    queue_.pop_front();
  }
  queue_.push_back(LogMessage{level, line});
  ready_.notify_one();
}

std::optional<LogMessage> AsyncLogBuffer::waitPopFor(
    std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  ready_.wait_for(lock, timeout, [&] {
    return !queue_.empty() || closed_;
  });
  if (queue_.empty()) {
    return std::nullopt;
  }
  LogMessage out = std::move(queue_.front());
  queue_.pop_front();
  return out;
}

}  // namespace evrp::sdk
