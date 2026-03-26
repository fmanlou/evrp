#pragma once

#include <cstdint>

namespace evrp::device::api {

// 与 proto DeviceKind 对齐。与 device.proto 的互转见 evrp/device/common/deviceprotoconv.h。
enum class DeviceKind {
  kUnspecified = 0,
  kTouchpad = 1,
  kTouchscreen = 2,
  kMouse = 3,
  kKeyboard = 4,
};

// 与 proto InputEvent 对齐。
struct InputEvent {
  DeviceKind device = DeviceKind::kUnspecified;
  int64_t time_sec = 0;
  int64_t time_usec = 0;
  uint32_t type = 0;
  uint32_t code = 0;
  int32_t value = 0;
};

}  // namespace evrp::device::api
