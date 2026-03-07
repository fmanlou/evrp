#pragma once

#include "evdev/evdev.h"

#include <ostream>
#include <string>
#include <vector>

struct RecordTarget {
  int fd;
  std::string label;
  std::string path;
};

struct keyboard_filter_state {
  int ctrl_down_count;
  bool saw_ctrl_c;
  std::vector<evdev::Event> pending_events;
};

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

struct touch_segment_decision {
  bool emit_break_before_event;
  bool emit_break_after_event;
};

bool is_touchpad(const char* dev_path);
bool is_mouse(const char* dev_path);
bool is_keyboard(const char* dev_path);

bool is_touchpad_from_capabilities(const evdev::Capabilities& caps);
bool is_mouse_from_capabilities(const evdev::Capabilities& caps);
bool is_keyboard_from_capabilities(const evdev::Capabilities& caps);
evdev::Event make_event(unsigned short type, unsigned short code, int value);
evdev::Event make_key_event(unsigned short code, int value);
void process_keyboard_event_with_ctrl_filter(
    const evdev::Event& ev, keyboard_filter_state* state,
    std::vector<evdev::Event>* emitted_events);
void flush_keyboard_event_filter(keyboard_filter_state* state,
                                 std::vector<evdev::Event>* emitted_events);
touch_segment_decision process_touch_event_for_segment(
    const evdev::Event& ev, touch_segment_state* state);

std::string find_first_touchpad();
std::string find_first_mouse();
std::string find_first_keyboard();

void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out);


