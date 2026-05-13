#include "evrp/sdk/tofromstring.h"

namespace evrp::device::api {

DeviceKind toKind(const std::string& label) {
  if (label == "keyboard") return DeviceKind::kKeyboard;
  if (label == "mouse") return DeviceKind::kMouse;
  if (label == "touchpad") return DeviceKind::kTouchpad;
  if (label == "touchscreen") return DeviceKind::kTouchscreen;
  return DeviceKind::kUnspecified;
}

void toKind(const std::string& label, DeviceKind* outKind) {
  if (!outKind) {
    return;
  }
  *outKind = toKind(label);
}

std::string toString(DeviceKind kind) {
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
