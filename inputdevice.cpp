#include "inputdevice.h"

#include "evdev/evdev.h"
#include "filesystem/filesystem.h"

#include <cctype>
#include <iostream>

static bool name_like_touchpad(const std::string& name) {
  std::string n = name;
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  return n.find("touchpad") != std::string::npos ||
         n.find("trackpad") != std::string::npos ||
         n.find("synaptics") != std::string::npos ||
         n.find("elan") != std::string::npos;
}

static bool name_like_mouse(const std::string& name) {
  std::string n = name;
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  return n.find("mouse") != std::string::npos ||
         n.find("trackball") != std::string::npos ||
         n.find("pointer") != std::string::npos;
}

static bool name_like_keyboard(const std::string& name) {
  std::string n = name;
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  return n.find("keyboard") != std::string::npos ||
         n.find("keypad") != std::string::npos;
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

bool is_touchpad_from_capabilities(const evdev::Capabilities& caps) {
  bool has_abs = caps.ev_abs && (caps.abs_x || caps.abs_mt_position_x);
  bool has_finger_tool =
      caps.btn_tool_finger || caps.btn_tool_doubletap || caps.btn_tool_tripletap;

  return has_abs && has_finger_tool && name_like_touchpad(caps.name);
}

bool is_mouse_from_capabilities(const evdev::Capabilities& caps) {
  bool has_rel = caps.ev_rel && caps.rel_x && caps.rel_y;
  bool has_buttons = caps.btn_left || caps.btn_right || caps.btn_middle;

  return has_rel && has_buttons && name_like_mouse(caps.name);
}

bool is_keyboard_from_capabilities(const evdev::Capabilities& caps) {
  bool has_keyboard_keys =
      caps.key_enter || caps.key_space || caps.key_esc || caps.key_a;

  return caps.ev_key && has_keyboard_keys && name_like_keyboard(caps.name);
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

void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out) {
  if (targets.empty()) return;
  FileSystem fs;

  evdev::signal_install_sigint();

  std::vector<int> fds;
  fds.reserve(targets.size());
  for (const auto& t : targets) {
    std::cout << "Recording " << t.label << " from " << t.path << std::endl;
    fds.push_back(t.fd);
  }
  std::cout << "(Ctrl+C to stop)" << std::endl;

  evdev::Event events[64];
  bool ready[32];
  while (!evdev::signal_stop_requested()) {
    int ret = fs.poll_fds(fds.data(), static_cast<int>(fds.size()), -1, ready);
    if (ret < 0) {
      if (evdev::errno_is_eintr() && evdev::signal_stop_requested()) break;
      fs.print_error("poll");
      break;
    }

    for (size_t i = 0; i < fds.size(); ++i) {
      if (!ready[i]) continue;

      int count = evdev::read_events(fds[i], events, 64);
      if (count < 0) {
        fs.print_error("read");
        break;
      }
      if (count == 0) continue;

      for (int j = 0; j < count; ++j) {
        const auto& ev = events[j];
        if (ev.type == evdev::EV_SYN) continue;

        event_out << "[" << targets[i].label << "] " << ev.sec << "."
                  << ev.usec << " type=" << ev.type << " code=" << ev.code
                  << " value=" << ev.value << "\n";
      }
    }
  }

  event_out.flush();
  evdev::signal_restore_sigint();
}

