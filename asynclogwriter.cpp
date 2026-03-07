#include "asynclogwriter.h"

#include <iostream>

AsyncLogWriter::AsyncLogWriter() : done_(false) {}

AsyncLogWriter::~AsyncLogWriter() { stop(); }

void AsyncLogWriter::start() {
  thread_ = std::thread(&AsyncLogWriter::run, this);
}

void AsyncLogWriter::push(const std::string& line) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(line);
  cv_.notify_one();
}

void AsyncLogWriter::stop() {
  done_ = true;
  if (thread_.joinable()) {
    cv_.notify_one();
    thread_.join();
  }
}

void AsyncLogWriter::run() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!done_ || !queue_.empty()) {
    cv_.wait(lock, [this]() { return done_ || !queue_.empty(); });
    while (!queue_.empty()) {
      std::string s = std::move(queue_.front());
      queue_.pop();
      lock.unlock();
      std::cout << s << std::endl;
      lock.lock();
    }
  }
}
