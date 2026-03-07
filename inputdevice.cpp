#include "inputdevice.h"

#include "evdev/evdev.h"
#include "filesystem/filesystem.h"
#include "keyboarddevice.h"
#include "touchdevice.h"

#include <cctype>
#include <string>

static bool name_like_mouse(const std::string& name) {
  std::string n = name;
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  return n.find("mouse") != std::string::npos ||
         n.find("trackball") != std::string::npos ||
         n.find("pointer") != std::string::npos;
}

bool is_touchpad(const char* dev_path) {
  evdev::Capabilities caps;
  if (!evdev::open_and_get_capabilities(dev_path, &caps)) return false;

  return is_touchpad_from_capabilities(caps);
}

bool is_mouse(const char* dev_path) {
  evdev::Capabilities caps;
  if (!evdev::open_and_get_capabilities(dev_path, &caps)) return false;

  return is_mouse_from_capabilities(caps);
}

bool is_keyboard(const char* dev_path) {
  evdev::Capabilities caps;
  if (!evdev::open_and_get_capabilities(dev_path, &caps)) return false;

  return is_keyboard_from_capabilities(caps);
}

bool is_mouse_from_capabilities(const evdev::Capabilities& caps) {
  bool has_rel = caps.ev_rel && caps.rel_x && caps.rel_y;
  bool has_buttons = caps.btn_left || caps.btn_right || caps.btn_middle;

  return has_rel && has_buttons && name_like_mouse(caps.name);
}

evdev::Event make_event(unsigned short type, unsigned short code, int value) {
  evdev::Event ev = {};
  ev.type = type;
  ev.code = code;
  ev.value = value;
  return ev;
}

std::string find_first_touchpad() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_touchpad(dev.c_str())) return dev;
  }
  return {};
}

std::string find_first_mouse() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_mouse(dev.c_str())) return dev;
  }
  return {};
}

std::string find_first_keyboard() {
  for (int i = 0; i < 32; ++i) {
    std::string dev = "/dev/input/event" + std::to_string(i);
    if (is_keyboard(dev.c_str())) return dev;
  }
  return {};
}
