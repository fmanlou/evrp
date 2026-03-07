#include "inputdevice.h"

#include "evdev/evdev.h"
#include "filesystem/filesystem.h"

#include <cctype>
#include <iostream>
#include <linux/input-event-codes.h>
#include <sstream>

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

static std::string key_name_from_code(unsigned short code) {
  if (code >= KEY_A && code <= KEY_Z) {
    char letter = static_cast<char>('A' + (code - KEY_A));
    return std::string(1, letter);
  }
  if (code >= KEY_1 && code <= KEY_9) {
    char digit = static_cast<char>('1' + (code - KEY_1));
    return std::string(1, digit);
  }
  if (code == KEY_0) return "0";

  switch (code) {
    case KEY_LEFTCTRL:
      return "LEFTCTRL";
    case KEY_RIGHTCTRL:
      return "RIGHTCTRL";
    case KEY_LEFTSHIFT:
      return "LEFTSHIFT";
    case KEY_RIGHTSHIFT:
      return "RIGHTSHIFT";
    case KEY_LEFTALT:
      return "LEFTALT";
    case KEY_RIGHTALT:
      return "RIGHTALT";
    case KEY_SPACE:
      return "SPACE";
    case KEY_ENTER:
      return "ENTER";
    case KEY_ESC:
      return "ESC";
    case KEY_TAB:
      return "TAB";
    case KEY_BACKSPACE:
      return "BACKSPACE";
    default:
      return "unknown(" + std::to_string(code) + ")";
  }
}

static std::string key_action_from_value(int value) {
  if (value == 0) return "release";
  if (value == 1) return "press";
  if (value == 2) return "repeat";
  return "unknown(" + std::to_string(value) + ")";
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
      oss << " // key=" << key_name_from_code(ev.code)
          << " action=" << key_action_from_value(ev.value);
    } else {
      oss << " // key=N/A action=non-key-event";
    }
  }
  return oss.str();
}

struct touch_segment_state {
  int current_slot;
  std::vector<char> slot_active;
  int active_mt_count;
  bool btn_touch_active;
  bool tool_finger_active;
  bool tool_doubletap_active;
  bool tool_tripletap_active;
  bool tool_quadtap_active;
  bool pending_segment_break;
};

static bool is_touching_now(const touch_segment_state& state) {
  bool any_tool_active = state.tool_finger_active || state.tool_doubletap_active ||
                         state.tool_tripletap_active || state.tool_quadtap_active;
  return state.active_mt_count > 0 || state.btn_touch_active || any_tool_active;
}

static bool update_touch_segment_state(const evdev::Event& ev, touch_segment_state* state) {
  if (!state) return false;

  bool was_touching = is_touching_now(*state);

  if (ev.type == EV_ABS) {
    if (ev.code == ABS_MT_SLOT) {
      state->current_slot = ev.value;
      if (state->current_slot >= 0 &&
          static_cast<size_t>(state->current_slot) >= state->slot_active.size()) {
        state->slot_active.resize(static_cast<size_t>(state->current_slot + 1), 0);
      }
    } else if (ev.code == ABS_MT_TRACKING_ID && state->current_slot >= 0) {
      if (static_cast<size_t>(state->current_slot) >= state->slot_active.size()) {
        state->slot_active.resize(static_cast<size_t>(state->current_slot + 1), 0);
      }
      char& current_active = state->slot_active[static_cast<size_t>(state->current_slot)];
      if (ev.value == -1) {
        if (current_active) {
          current_active = 0;
          state->active_mt_count -= 1;
        }
      } else {
        if (!current_active) {
          current_active = 1;
          state->active_mt_count += 1;
        }
      }
    }
  } else if (ev.type == EV_KEY) {
    if (ev.code == BTN_TOUCH) state->btn_touch_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_FINGER) state->tool_finger_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_DOUBLETAP) state->tool_doubletap_active = (ev.value != 0);
    if (ev.code == BTN_TOOL_TRIPLETAP) state->tool_tripletap_active = (ev.value != 0);
#ifdef BTN_TOOL_QUADTAP
    if (ev.code == BTN_TOOL_QUADTAP) state->tool_quadtap_active = (ev.value != 0);
#endif
  }

  bool is_touching = is_touching_now(*state);
  return was_touching && !is_touching;
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

evdev::Event make_key_event(unsigned short code, int value) {
  evdev::Event ev = {};
  ev.type = EV_KEY;
  ev.code = code;
  ev.value = value;
  return ev;
}

void process_keyboard_event_with_ctrl_filter(
    const evdev::Event& ev, keyboard_filter_state* state,
    std::vector<evdev::Event>* emitted_events) {
  if (!state || !emitted_events) return;

  if (ev.type == EV_KEY) {
    bool is_ctrl_key = (ev.code == KEY_LEFTCTRL || ev.code == KEY_RIGHTCTRL);
    bool is_press = (ev.value == 1);
    bool is_repeat = (ev.value == 2);
    bool is_release = (ev.value == 0);

    if (is_ctrl_key) {
      if (is_press) {
        state->ctrl_down_count += 1;
      }
      state->pending_events.push_back(ev);
      if (is_release && state->ctrl_down_count > 0) {
        state->ctrl_down_count -= 1;
      }
      if (is_release && state->ctrl_down_count == 0) {
        if (!state->saw_ctrl_c) {
          emitted_events->insert(emitted_events->end(), state->pending_events.begin(),
                                 state->pending_events.end());
        }
        state->saw_ctrl_c = false;
        state->pending_events.clear();
      }
      return;
    }

    if (state->ctrl_down_count > 0) {
      state->pending_events.push_back(ev);
      if (ev.code == KEY_C && (is_press || is_repeat)) {
        state->saw_ctrl_c = true;
      }
      return;
    }
  } else if (state->ctrl_down_count > 0) {
    state->pending_events.push_back(ev);
    return;
  }

  if (!state->pending_events.empty() && state->ctrl_down_count == 0) {
    if (!state->saw_ctrl_c) {
      emitted_events->insert(emitted_events->end(), state->pending_events.begin(),
                             state->pending_events.end());
    }
    state->saw_ctrl_c = false;
    state->pending_events.clear();
  }

  emitted_events->push_back(ev);
}

void flush_keyboard_event_filter(keyboard_filter_state* state,
                                 std::vector<evdev::Event>* emitted_events) {
  if (!state || !emitted_events) return;
  if (state->ctrl_down_count != 0) return;
  if (!state->saw_ctrl_c && !state->pending_events.empty()) {
    emitted_events->insert(emitted_events->end(), state->pending_events.begin(),
                           state->pending_events.end());
  }
  state->saw_ctrl_c = false;
  state->pending_events.clear();
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
            event_out << format_event_line(targets[i].label, out_ev) << "\n";
          }
          continue;
        }

        if (targets[i].label == "touchpad") {
          touch_segment_state& touch_state = touch_states[i];
          bool is_msc_timestamp = (ev.type == EV_MSC && ev.code == MSC_TIMESTAMP);

          if (touch_state.pending_segment_break && !is_msc_timestamp) {
            event_out << "\n";
            touch_state.pending_segment_break = false;
          }

          event_out << format_event_line(targets[i].label, ev) << "\n";
          bool segment_ended = update_touch_segment_state(ev, &touch_state);
          if (segment_ended) {
            touch_state.pending_segment_break = true;
          }

          if (touch_state.pending_segment_break && is_msc_timestamp) {
            event_out << "\n";
            touch_state.pending_segment_break = false;
          }
          continue;
        }

        event_out << format_event_line(targets[i].label, ev) << "\n";
      }
    }
  }

  for (size_t i = 0; i < touch_states.size(); ++i) {
    if (touch_states[i].pending_segment_break) {
      event_out << "\n";
      touch_states[i].pending_segment_break = false;
    }
  }

  event_out.flush();
  evdev::signal_restore_sigint();
}

