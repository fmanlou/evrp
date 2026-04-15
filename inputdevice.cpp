#include "inputdevice.h"

#include <cctype>
#include <string>

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

std::string findFirstTouchpad() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (isTouchpad(dev.c_str())) return dev;
  }
  return {};
}

std::string findFirstTouchscreen() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (isTouchscreen(dev.c_str())) return dev;
  }
  return {};
}

std::string findFirstMouse() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (isMouse(dev.c_str())) return dev;
  }
  return {};
}

std::string findFirstKeyboard() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (isKeyboard(dev.c_str())) return dev;
  }
  return {};
}

std::string findDevicePath(evrp::device::api::DeviceKind kind) {
  using evrp::device::api::DeviceKind;
  switch (kind) {
    case DeviceKind::kTouchpad:
      return findFirstTouchpad();
    case DeviceKind::kTouchscreen:
      return findFirstTouchscreen();
    case DeviceKind::kMouse:
      return findFirstMouse();
    case DeviceKind::kKeyboard:
      return findFirstKeyboard();
    case DeviceKind::kUnspecified:
    default:
      return "";
  }
}
