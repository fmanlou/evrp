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

bool isTouchpad(const char *dev_path) {
  Capabilities caps;
  if (!openAndGetCapabilities(dev_path, &caps)) return false;

  return isTouchpadFromCapabilities(caps);
}

bool isTouchscreen(const char *dev_path) {
  Capabilities caps;
  if (!openAndGetCapabilities(dev_path, &caps)) return false;

  return isTouchscreenFromCapabilities(caps);
}

bool isMouse(const char *dev_path) {
  Capabilities caps;
  if (!openAndGetCapabilities(dev_path, &caps)) return false;

  return isMouseFromCapabilities(caps);
}

bool isKeyboard(const char *dev_path) {
  Capabilities caps;
  if (!openAndGetCapabilities(dev_path, &caps)) return false;

  return isKeyboardFromCapabilities(caps);
}

bool isMouseFromCapabilities(const Capabilities &caps) {
  bool has_rel = caps.ev_rel && caps.rel_x && caps.rel_y;
  bool has_buttons = caps.btn_left || caps.btn_right || caps.btn_middle;

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

std::string findDevicePath(DeviceId id) {
  switch (id) {
    case DeviceId::Touchpad:
      return findFirstTouchpad();
    case DeviceId::Touchscreen:
      return findFirstTouchscreen();
    case DeviceId::Mouse:
      return findFirstMouse();
    case DeviceId::Keyboard:
      return findFirstKeyboard();
    case DeviceId::Unknown:
      return "";
  }
  return "";
}
