#pragma once

#include <string>

namespace logging {
class LogService;
}

extern logging::LogService* logService;

#include "log/common/logservice.h"

logging::LogLevel logLevelFromString(const std::string& s);
const char* logLevelName(logging::LogLevel level);
