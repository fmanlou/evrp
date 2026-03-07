#pragma once

#include "evdev/evdev.h"

#include <string>
#include <vector>

struct keyboard_filter_state {
  int ctrl_down_count;
  bool saw_ctrl_c;
  std::vector<evdev::Event> pending_events;
};

evdev::Event make_key_event(unsigned short code, int value);
std::string keyboard_key_name_from_code(unsigned short code);
std::string keyboard_key_action_from_value(int value);
void process_keyboard_event_with_ctrl_filter(
    const evdev::Event& ev, keyboard_filter_state* state,
    std::vector<evdev::Event>* emitted_events);
void flush_keyboard_event_filter(keyboard_filter_state* state,
                                 std::vector<evdev::Event>* emitted_events);
bool is_keyboard_from_capabilities(const evdev::Capabilities& caps);

