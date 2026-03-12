#include "deviceid.h"

#include <string>

DeviceId device_id_from_label(const std::string &label) {
  if (label == "keyboard") return DeviceId::Keyboard;
  if (label == "mouse") return DeviceId::Mouse;
  if (label == "touchpad") return DeviceId::Touchpad;
  if (label == "touchscreen") return DeviceId::Touchscreen;
  return DeviceId::Unknown;
}

std::string device_label(DeviceId id) {
  switch (id) {
    case DeviceId::Keyboard:
      return "keyboard";
    case DeviceId::Mouse:
      return "mouse";
    case DeviceId::Touchpad:
      return "touchpad";
    case DeviceId::Touchscreen:
      return "touchscreen";
    case DeviceId::Unknown:
      return "";
  }
  return "";
}
