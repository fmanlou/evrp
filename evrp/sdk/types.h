#pragma once

#include <cstdint>
#include <string>

namespace evrp::sdk {

enum class DeviceKind {
  kUnspecified = 0,
  kTouchpad = 1,
  kTouchscreen = 2,
  kMouse = 3,
  kKeyboard = 4,
};

struct InputEvent {
  DeviceKind device = DeviceKind::kUnspecified;
  int64_t timeSec = 0;
  int64_t timeUsec = 0;
  uint32_t type = 0;
  uint32_t code = 0;
  int32_t value = 0;
};

struct StatusCode {
  int32_t code = 0;
  std::string message;
};

}  // namespace evrp::sdk
