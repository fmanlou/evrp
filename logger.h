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

LogLevel logLevelFromString(const std::string& s);
const char* logLevelName(LogLevel level);

namespace logging {
class LogService;
}

class Logger {
 public:
  Logger();
  ~Logger();

  void setLevel(LogLevel level);
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

inline void logError(const std::string& msg) {
  if (g_logger) g_logger->error(msg);
}
inline void logWarn(const std::string& msg) {
  if (g_logger) g_logger->warn(msg);
}
inline void logInfo(const std::string& msg) {
  if (g_logger) g_logger->info(msg);
}
inline void logDebug(const std::string& msg) {
  if (g_logger) g_logger->debug(msg);
}
inline void logTrace(const std::string& msg) {
  if (g_logger) g_logger->trace(msg);
}
