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
  // Shown in log lines to identify the process (e.g. "evrp-device", "evrp").
  explicit Logger(std::string name);
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

extern Logger* gLogger;

inline void logError(const std::string& msg) {
  if (gLogger) gLogger->error(msg);
}
inline void logWarn(const std::string& msg) {
  if (gLogger) gLogger->warn(msg);
}
inline void logInfo(const std::string& msg) {
  if (gLogger) gLogger->info(msg);
}
inline void logDebug(const std::string& msg) {
  if (gLogger) gLogger->debug(msg);
}
inline void logTrace(const std::string& msg) {
  if (gLogger) gLogger->trace(msg);
}
