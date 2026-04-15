#pragma once

#include <string>
#include <vector>

#include "evdev.h"

struct keyboard_filter_state {
  int ctrl_down_count;
  bool saw_ctrl_c;
  std::vector<Event> pending_events;
};

Event makeKeyEvent(unsigned short code, int value);
std::string keyboardKeyNameFromCode(unsigned short code);
std::string keyboardKeyActionFromValue(int value);
void processKeyboardEventWithCtrlFilter(
    const Event &ev, keyboard_filter_state *state,
    std::vector<Event> *emittedEvents);
void flushKeyboardEventFilter(keyboard_filter_state *state,
                                 std::vector<Event> *emittedEvents);
bool isKeyboardFromCapabilities(const Capabilities &caps);
