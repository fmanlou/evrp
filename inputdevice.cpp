#include "inputdevice.h"

#include <cctype>
#include <string>
#include <vector>

#include "evdev.h"
#include "filesystem.h"
#include "keyboard/keyboarddevice.h"
#include "touchdevice.h"

static bool nameLikeMouse(const std::string &name) {
  std::string n = name;
  for (auto &c : n) c = static_cast<char>(std::tolower(c));
  return n.find("mouse") != std::string::npos ||
         n.find("trackball") != std::string::npos ||
         n.find("pointer") != std::string::npos;
}

bool isTouchpad(const char *devPath) {
  Capabilities caps;
  if (!openAndGetCapabilities(devPath, &caps)) return false;

  return isTouchpadFromCapabilities(caps);
}

bool isTouchscreen(const char *devPath) {
  Capabilities caps;
  if (!openAndGetCapabilities(devPath, &caps)) return false;

  return isTouchscreenFromCapabilities(caps);
}

bool isMouse(const char *devPath) {
  Capabilities caps;
  if (!openAndGetCapabilities(devPath, &caps)) return false;

  return isMouseFromCapabilities(caps);
}

bool isKeyboard(const char *devPath) {
  Capabilities caps;
  if (!openAndGetCapabilities(devPath, &caps)) return false;

  return isKeyboardFromCapabilities(caps);
}

bool isMouseFromCapabilities(const Capabilities &caps) {
  bool has_rel = caps.evRel && caps.relX && caps.relY;
  bool has_buttons = caps.btnLeft || caps.btnRight || caps.btnMiddle;

  return has_rel && has_buttons && nameLikeMouse(caps.name);
}

Event makeEvent(unsigned short type, unsigned short code, int value) {
  Event ev = {};
  ev.type = type;
  ev.code = code;
  ev.value = value;
  return ev;
}

std::vector<std::string> findAllDevicePaths(
    evrp::device::api::DeviceKind kind) {
  using evrp::device::api::DeviceKind;
  std::vector<std::string> out;
  auto pushIf = [&](bool (*pred)(const char*)) {
    for (int i = 0; i < 32; ++i) {
      std::string dev = "/dev/input/event" + std::to_string(i);
      if (pred(dev.c_str())) {
        out.push_back(std::move(dev));
      }
    }
  };
  switch (kind) {
    case DeviceKind::kTouchpad:
      pushIf(isTouchpad);
      break;
    case DeviceKind::kTouchscreen:
      pushIf(isTouchscreen);
      break;
    case DeviceKind::kMouse:
      pushIf(isMouse);
      break;
    case DeviceKind::kKeyboard:
      pushIf(isKeyboard);
      break;
    case DeviceKind::kUnspecified:
    default:
      break;
  }
  return out;
}

std::string findDevicePath(evrp::device::api::DeviceKind kind) {
  const std::vector<std::string> all = findAllDevicePaths(kind);
  if (all.empty()) {
    return {};
  }
  return all.front();
}

std::string findFirstTouchpad() {
  return findDevicePath(evrp::device::api::DeviceKind::kTouchpad);
}

std::string findFirstTouchscreen() {
  return findDevicePath(evrp::device::api::DeviceKind::kTouchscreen);
}

std::string findFirstMouse() {
  return findDevicePath(evrp::device::api::DeviceKind::kMouse);
}

std::string findFirstKeyboard() {
  return findDevicePath(evrp::device::api::DeviceKind::kKeyboard);
}
