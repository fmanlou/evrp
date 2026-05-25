#pragma once

#include <string>

namespace logging {
class ILogService;
}

extern logging::ILogService* logService;

#include "log/common/logservice.h"

logging::LogLevel logLevelFromString(const std::string& s);
const char* logLevelName(logging::LogLevel level);
