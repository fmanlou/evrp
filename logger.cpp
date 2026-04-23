#include "logger.h"

#include <algorithm>
#include <cctype>

logging::LogService* logService = nullptr;

logging::LogLevel logLevelFromString(const std::string& s) {
  std::string lower = s;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (lower == "error") return logging::LogLevel::Error;
  if (lower == "warn" || lower == "warning") return logging::LogLevel::Warning;
  if (lower == "info") return logging::LogLevel::Info;
  if (lower == "debug") return logging::LogLevel::Debug;
  if (lower == "trace") return logging::LogLevel::Trace;
  if (lower == "off") return logging::LogLevel::Off;
  return logging::LogLevel::Info;
}

const char* logLevelName(logging::LogLevel level) {
  switch (level) {
    case logging::LogLevel::Off:
      return "OFF";
    case logging::LogLevel::Error:
      return "ERROR";
    case logging::LogLevel::Warning:
      return "WARN";
    case logging::LogLevel::Info:
      return "INFO";
    case logging::LogLevel::Debug:
      return "DEBUG";
    case logging::LogLevel::Trace:
      return "TRACE";
  }
  return "INFO";
}
