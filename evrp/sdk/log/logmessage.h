#pragma once

#include <string>

#include "log/common/log.h"

namespace evrp::sdk {

struct LogMessage {
  logging::LogLevel level{logging::LogLevel::Info};
  std::string text;
};

}  // namespace evrp::sdk
