#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <string>

#include "evrp/sdk/log/logmessage.h"
#include "log/common/log.h"

namespace evrp::sdk {

class AsyncLogBuffer {
 public:
  static constexpr std::size_t kMaxQueue = 2000;

  ~AsyncLogBuffer();

  void push(logging::LogLevel level, const std::string& line);

  std::optional<LogMessage> waitPopFor(std::chrono::milliseconds timeout);

 private:
  std::mutex mutex_;
  std::condition_variable ready_;
  std::deque<LogMessage> queue_;
  bool closed_{false};
};

}  // namespace evrp::sdk
