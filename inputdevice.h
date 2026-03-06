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

bool is_touchpad(const char* dev_path);
bool is_mouse(const char* dev_path);
bool is_keyboard(const char* dev_path);

bool is_touchpad_from_capabilities(const evdev::Capabilities& caps);
bool is_mouse_from_capabilities(const evdev::Capabilities& caps);
bool is_keyboard_from_capabilities(const evdev::Capabilities& caps);
evdev::Event make_key_event(unsigned short code, int value);
void process_keyboard_event_with_ctrl_filter(
    const evdev::Event& ev, keyboard_filter_state* state,
    std::vector<evdev::Event>* emitted_events);
void flush_keyboard_event_filter(keyboard_filter_state* state,
                                 std::vector<evdev::Event>* emitted_events);

std::string find_first_touchpad();
std::string find_first_mouse();
std::string find_first_keyboard();

void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out);


