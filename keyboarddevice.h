#pragma once

#include <string>
#include <vector>

#include "evdev.h"

struct keyboard_filter_state {
  int ctrl_down_count;
  bool saw_ctrl_c;
  std::vector<Event> pending_events;
};

Event make_key_event(unsigned short code, int value);
std::string keyboard_key_name_from_code(unsigned short code);
std::string keyboard_key_action_from_value(int value);
void process_keyboard_event_with_ctrl_filter(
    const Event &ev, keyboard_filter_state *state,
    std::vector<Event> *emitted_events);
void flush_keyboard_event_filter(keyboard_filter_state *state,
                                 std::vector<Event> *emitted_events);
bool is_keyboard_from_capabilities(const Capabilities &caps);
