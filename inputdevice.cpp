#include "inputdevice.h"

#include "evdev/evdev.h"
#include "filesystem/filesystem.h"
#include "keyboarddevice.h"
#include "touchdevice.h"

#include <cctype>
#include <iostream>
#include <linux/input-event-codes.h>
#include <sstream>

static bool name_like_mouse(const std::string& name) {
  std::string n = name;
  for (auto& c : n) c = static_cast<char>(std::tolower(c));
  return n.find("mouse") != std::string::npos ||
         n.find("trackball") != std::string::npos ||
         n.find("pointer") != std::string::npos;
}

static std::string event_type_name(unsigned short type) {
  switch (type) {
    case EV_SYN:
      return "EV_SYN";
    case EV_KEY:
      return "EV_KEY";
    case EV_REL:
      return "EV_REL";
    case EV_ABS:
      return "EV_ABS";
    case EV_MSC:
      return "EV_MSC";
    default:
      return "EV_UNKNOWN";
  }
}

static std::string event_code_name(unsigned short type, unsigned short code) {
  if (type == EV_MSC) {
    switch (code) {
      case MSC_SCAN:
        return "MSC_SCAN";
      case MSC_TIMESTAMP:
        return "MSC_TIMESTAMP";
      default:
        return "";
    }
  }
  if (type == EV_SYN) {
    switch (code) {
      case SYN_REPORT:
        return "SYN_REPORT";
      case SYN_MT_REPORT:
        return "SYN_MT_REPORT";
      default:
        return "";
    }
  }
  return "";
}

static std::string format_event_line(const std::string& label, const evdev::Event& ev) {
  std::ostringstream oss;
  std::string code_name = event_code_name(ev.type, ev.code);
  oss << "[" << label << "] " << ev.sec << "." << ev.usec << " type=" << ev.type
      << "(" << event_type_name(ev.type) << ")"
      << " code=" << ev.code;
  if (!code_name.empty()) {
    oss << "(" << code_name << ")";
  }
  oss << " value=" << ev.value;
  if (label == "keyboard") {
    if (ev.type == EV_KEY) {
      oss << " // key=" << keyboard_key_name_from_code(ev.code)
          << " action=" << keyboard_key_action_from_value(ev.value);
    } else {
      oss << " // key=N/A action=non-key-event";
    }
  }
  return oss.str();
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

void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out,
                         std::ostream* console_out) {
  if (targets.empty()) return;
  FileSystem fs;

  evdev::signal_install_sigint();

  std::vector<int> fds;
  fds.reserve(targets.size());
  for (const auto& t : targets) {
    if (console_out) {
      *console_out << "Recording " << t.label << " from " << t.path << std::endl;
    }
    fds.push_back(t.fd);
  }
  if (console_out) {
    *console_out << "(Ctrl+C to stop)" << std::endl;
  }

  auto write_line = [&](const std::string& line) {
    event_out << line << "\n";
    if (console_out) *console_out << line << "\n";
  };
  auto write_newline = [&]() {
    event_out << "\n";
    if (console_out) *console_out << "\n";
  };

  evdev::Event events[64];
  bool ready[32];
  std::vector<keyboard_filter_state> keyboard_states(fds.size());
  std::vector<touch_segment_state> touch_states(fds.size());
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
        if (ev.type == EV_SYN) continue;

        if (targets[i].label == "keyboard") {
          std::vector<evdev::Event> emitted_events;
          process_keyboard_event_with_ctrl_filter(ev, &keyboard_states[i],
                                                  &emitted_events);
          for (const auto& out_ev : emitted_events) {
            write_line(format_event_line(targets[i].label, out_ev));
          }
          continue;
        }

        if (targets[i].label == "touchpad") {
          touch_segment_state& touch_state = touch_states[i];
          touch_segment_decision decision =
              process_touch_event_for_segment(ev, &touch_state);
          if (decision.emit_break_before_event) {
            write_newline();
          }

          write_line(format_event_line(targets[i].label, ev));
          if (decision.emit_break_after_event) {
            write_newline();
          }
          continue;
        }

        write_line(format_event_line(targets[i].label, ev));
      }
    }
  }

  for (size_t i = 0; i < touch_states.size(); ++i) {
    if (touch_states[i].pending_segment_break) {
      write_newline();
      touch_states[i].pending_segment_break = false;
    }
  }

  event_out.flush();
  evdev::signal_restore_sigint();
}

