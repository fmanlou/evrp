#pragma once

#include <memory>
#include <string>

enum class LogLevel {
  Error = 0,
  Warn = 1,
  Info = 2,
  Debug = 3,
  Trace = 4,
};

LogLevel log_level_from_string(const std::string& s);
const char* log_level_name(LogLevel level);

namespace logging {
class LogService;
}

class Logger {
 public:
  Logger();
  ~Logger();

  void set_level(LogLevel level);
  LogLevel level() const;

  void error(const std::string& msg);
  void warn(const std::string& msg);
  void info(const std::string& msg);
  void debug(const std::string& msg);
  void trace(const std::string& msg);

  void log(LogLevel level, const std::string& msg);

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

 private:
  std::unique_ptr<logging::LogService> service_;
};

extern Logger* g_logger;

inline void log_error(const std::string& msg) {
  if (g_logger) g_logger->error(msg);
}
inline void log_warn(const std::string& msg) {
  if (g_logger) g_logger->warn(msg);
}
inline void log_info(const std::string& msg) {
  if (g_logger) g_logger->info(msg);
}
inline void log_debug(const std::string& msg) {
  if (g_logger) g_logger->debug(msg);
}
inline void log_trace(const std::string& msg) {
  if (g_logger) g_logger->trace(msg);
}
