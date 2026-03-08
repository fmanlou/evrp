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

Logger::Logger() : level_(LogLevel::Info) {}

Logger::~Logger() {}

Logger *g_logger = nullptr;

bool Logger::should_log(LogLevel level) const {
  return static_cast<int>(level) <= static_cast<int>(level_);
}

void Logger::log(LogLevel level, const std::string &msg) {
  if (!should_log(level)) return;
  std::ostringstream oss;
  oss << "[" << log_level_name(level) << "] " << msg;
  std::string line = oss.str();
  if (level == LogLevel::Error || level == LogLevel::Warn) {
    std::cerr << line << std::endl;
  } else {
    std::cout << line << std::endl;
  }
}

void Logger::error(const std::string &msg) { log(LogLevel::Error, msg); }

void Logger::warn(const std::string &msg) { log(LogLevel::Warn, msg); }

void Logger::info(const std::string &msg) { log(LogLevel::Info, msg); }

void Logger::debug(const std::string &msg) { log(LogLevel::Debug, msg); }

void Logger::trace(const std::string &msg) { log(LogLevel::Trace, msg); }

void Logger::push(const std::string &line) {
  if (!should_log(LogLevel::Debug)) return;
  std::cout << line << std::endl;
}
