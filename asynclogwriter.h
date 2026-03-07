#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class AsyncLogWriter {
 public:
  AsyncLogWriter();
  ~AsyncLogWriter();

  void start();
  void push(const std::string& line);
  void stop();

 private:
  void run();

  std::queue<std::string> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> done_;
  std::thread thread_;
};
