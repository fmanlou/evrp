#pragma once

#include <string>

enum class LogLevel {
  Error = 0,
  Warn = 1,
  Info = 2,
  Debug = 3,
  Trace = 4,
};

LogLevel log_level_from_string(const std::string &s);
const char *log_level_name(LogLevel level);

class Logger {
 public:
  void set_level(LogLevel level) { level_ = level; }
  LogLevel level() const { return level_; }

  void error(const std::string &msg);
  void warn(const std::string &msg);
  void info(const std::string &msg);
  void debug(const std::string &msg);
  void trace(const std::string &msg);

  // Push raw line at DEBUG level (for event logs, no prefix).
  void push(const std::string &line);

  void log(LogLevel level, const std::string &msg);

  Logger();
  ~Logger();
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

 private:
  bool should_log(LogLevel level) const;

  LogLevel level_;
};

extern Logger *g_logger;

inline void log_error(const std::string &msg) {
  if (g_logger) g_logger->error(msg);
}
inline void log_warn(const std::string &msg) {
  if (g_logger) g_logger->warn(msg);
}
inline void log_info(const std::string &msg) {
  if (g_logger) g_logger->info(msg);
}
inline void log_debug(const std::string &msg) {
  if (g_logger) g_logger->debug(msg);
}
inline void log_trace(const std::string &msg) {
  if (g_logger) g_logger->trace(msg);
}
inline void log_push(const std::string &line) {
  if (g_logger) g_logger->push(line);
}
