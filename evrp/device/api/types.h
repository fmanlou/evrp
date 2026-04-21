#pragma once

#include <cstdint>
#include <string>

namespace evrp::device::api {

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

struct OperationResult {
  int32_t code = 0;
  std::string message;
};

inline DeviceKind deviceKindFromLabel(const std::string& label) {
  if (label == "keyboard") return DeviceKind::kKeyboard;
  if (label == "mouse") return DeviceKind::kMouse;
  if (label == "touchpad") return DeviceKind::kTouchpad;
  if (label == "touchscreen") return DeviceKind::kTouchscreen;
  return DeviceKind::kUnspecified;
}

inline std::string deviceKindLabel(DeviceKind kind) {
  switch (kind) {
    case DeviceKind::kKeyboard:
      return "keyboard";
    case DeviceKind::kMouse:
      return "mouse";
    case DeviceKind::kTouchpad:
      return "touchpad";
    case DeviceKind::kTouchscreen:
      return "touchscreen";
    case DeviceKind::kUnspecified:
    default:
      return "";
  }
}

}  // namespace evrp::device::api
