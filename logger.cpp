#include "logger.h"

#include "log/common/logservice.h"

#include <algorithm>
#include <cctype>

namespace {

logging::LogLevel toLoggingLevel(LogLevel l) {
  switch (l) {
    case LogLevel::Error:
      return logging::LogLevel::Error;
    case LogLevel::Warn:
      return logging::LogLevel::Warning;
    case LogLevel::Info:
      return logging::LogLevel::Info;
    case LogLevel::Debug:
      return logging::LogLevel::Debug;
    case LogLevel::Trace:
      return logging::LogLevel::Trace;
  }
  return logging::LogLevel::Info;
}

LogLevel fromLoggingLevel(logging::LogLevel l) {
  switch (l) {
    case logging::LogLevel::Error:
      return LogLevel::Error;
    case logging::LogLevel::Warning:
      return LogLevel::Warn;
    case logging::LogLevel::Info:
      return LogLevel::Info;
    case logging::LogLevel::Debug:
      return LogLevel::Debug;
    case logging::LogLevel::Trace:
      return LogLevel::Trace;
    case logging::LogLevel::Off:
    default:
      return LogLevel::Info;
  }
}

}  

LogLevel logLevelFromString(const std::string& s) {
  std::string lower = s;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (lower == "error") return LogLevel::Error;
  if (lower == "warn" || lower == "warning") return LogLevel::Warn;
  if (lower == "info") return LogLevel::Info;
  if (lower == "debug") return LogLevel::Debug;
  if (lower == "trace") return LogLevel::Trace;
  return LogLevel::Info;
}

const char* logLevelName(LogLevel level) {
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

Logger::Logger(std::string name)
    : service_(std::make_unique<logging::LogService>(std::move(name))) {}

Logger::~Logger() = default;

void Logger::setLevel(LogLevel level) {
  service_->setLevel(toLoggingLevel(level));
}

LogLevel Logger::level() const {
  return fromLoggingLevel(service_->getLevel());
}

void Logger::log(LogLevel level, const std::string& msg) {
  service_->log(toLoggingLevel(level), std::string(msg));
}

void Logger::error(const std::string& msg) {
  log(LogLevel::Error, msg);
}

void Logger::warn(const std::string& msg) {
  log(LogLevel::Warn, msg);
}

void Logger::info(const std::string& msg) {
  log(LogLevel::Info, msg);
}

void Logger::debug(const std::string& msg) {
  log(LogLevel::Debug, msg);
}

void Logger::trace(const std::string& msg) {
  log(LogLevel::Trace, msg);
}

Logger* gLogger = nullptr;
