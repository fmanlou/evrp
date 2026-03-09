#include "logger.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <sstream>

LogLevel log_level_from_string(const std::string &s) {
  std::string lower = s;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c) { return std::tolower(c); });
  if (lower == "error") return LogLevel::Error;
  if (lower == "warn" || lower == "warning") return LogLevel::Warn;
  if (lower == "info") return LogLevel::Info;
  if (lower == "debug") return LogLevel::Debug;
  if (lower == "trace") return LogLevel::Trace;
  return LogLevel::Info;
}

const char *log_level_name(LogLevel level) {
  switch (level) {
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Warn:
      return "WARN";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Trace:
      return "TRACE";
  }
  return "INFO";
}

Logger::Logger() : level_(LogLevel::Info), worker_(&Logger::worker_loop, this) {}

Logger::~Logger() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
  }
  cv_.notify_one();
  if (worker_.joinable()) worker_.join();
}

Logger *g_logger = nullptr;

bool Logger::should_log(LogLevel level) const {
  return static_cast<int>(level) <= static_cast<int>(level_);
}

void Logger::enqueue(bool use_stderr, const std::string &line) {
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.emplace(use_stderr, line);
  cv_.notify_one();
}

void Logger::worker_loop() {
  while (true) {
    std::pair<bool, std::string> item;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] {
        return shutdown_ || !queue_.empty();
      });
      if (shutdown_ && queue_.empty()) return;
      if (queue_.empty()) continue;
      item = std::move(queue_.front());
      queue_.pop();
    }
    if (item.first) {
      std::cerr << item.second << std::endl;
    } else {
      std::cout << item.second << std::endl;
    }
  }
}

void Logger::log(LogLevel level, const std::string &msg) {
  if (!should_log(level)) return;
  std::ostringstream oss;
  oss << "[" << log_level_name(level) << "] " << msg;
  bool use_stderr =
      (level == LogLevel::Error || level == LogLevel::Warn);
  enqueue(use_stderr, oss.str());
}

void Logger::error(const std::string &msg) { log(LogLevel::Error, msg); }

void Logger::warn(const std::string &msg) { log(LogLevel::Warn, msg); }

void Logger::info(const std::string &msg) { log(LogLevel::Info, msg); }

void Logger::debug(const std::string &msg) { log(LogLevel::Debug, msg); }

void Logger::trace(const std::string &msg) { log(LogLevel::Trace, msg); }
