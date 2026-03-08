#include "keyboard/keyboarddevice.h"

#include <linux/input-event-codes.h>

#include <cctype>
#include <string>

static bool name_like_keyboard(const std::string &name) {
  std::string n = name;
  for (auto &c : n) c = static_cast<char>(std::tolower(c));
  return n.find("keyboard") != std::string::npos ||
         n.find("keypad") != std::string::npos;
}

bool is_keyboard_from_capabilities(const Capabilities &caps) {
  bool has_keyboard_keys =
      caps.key_enter || caps.key_space || caps.key_esc || caps.key_a;

  return caps.ev_key && has_keyboard_keys && name_like_keyboard(caps.name);
}

Event make_key_event(unsigned short code, int value) {
  Event ev = {};
  ev.type = EV_KEY;
  ev.code = code;
  ev.value = value;
  return ev;
}

std::string keyboard_key_name_from_code(unsigned short code) {
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

std::string keyboard_key_action_from_value(int value) {
  if (value == 0) return "release";
  if (value == 1) return "press";
  if (value == 2) return "repeat";
  return "unknown(" + std::to_string(value) + ")";
}

void process_keyboard_event_with_ctrl_filter(
    const Event &ev, keyboard_filter_state *state,
    std::vector<Event> *emitted_events) {
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
          emitted_events->insert(emitted_events->end(),
                                 state->pending_events.begin(),
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
      emitted_events->insert(emitted_events->end(),
                             state->pending_events.begin(),
                             state->pending_events.end());
    }
    state->saw_ctrl_c = false;
    state->pending_events.clear();
  }

  emitted_events->push_back(ev);
}

void flush_keyboard_event_filter(keyboard_filter_state *state,
                                 std::vector<Event> *emitted_events) {
  if (!state || !emitted_events) return;
  if (state->ctrl_down_count != 0) return;
  if (!state->saw_ctrl_c && !state->pending_events.empty()) {
    emitted_events->insert(emitted_events->end(), state->pending_events.begin(),
                           state->pending_events.end());
  }
  state->saw_ctrl_c = false;
  state->pending_events.clear();
}
