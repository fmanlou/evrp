#pragma once

#include <string>
#include <vector>

#include "evrp/sdk/evdev.h"

/// How to treat Ctrl+C chords while recording keyboard events.
enum class KeyboardCtrlCFilterMode {
  kOff = 0,       ///< Pass all events through.
  kFull,          ///< Drop the entire Ctrl-held window once Ctrl+C is detected.
  kEndingOnly,    ///< Drop KEY_C release and Ctrl release after Ctrl+C press.
};

KeyboardCtrlCFilterMode keyboardCtrlCFilterModeFromLabel(
    const std::string &label);

struct keyboard_filter_state {
  int ctrl_down_count;
  bool saw_ctrl_c;
  std::vector<Event> pending_events;
};

Event makeKeyEvent(unsigned short code, int value);
std::string keyboardKeyNameFromCode(unsigned short code);
std::string keyboardKeyActionFromValue(int value);
void processKeyboardEventWithCtrlFilter(
    const Event &ev, KeyboardCtrlCFilterMode mode,
    keyboard_filter_state *state, std::vector<Event> *emittedEvents);
void flushKeyboardEventFilter(KeyboardCtrlCFilterMode mode,
                              keyboard_filter_state *state,
                              std::vector<Event> *emittedEvents);
bool isKeyboardFromCapabilities(const Capabilities &caps);
