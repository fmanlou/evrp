#ifndef EVRP_INPUTDEVICE_H
#define EVRP_INPUTDEVICE_H

#include "evdev/evdev.h"

#include <ostream>
#include <string>
#include <vector>

struct RecordTarget {
  int fd;
  std::string label;
  std::string path;
};

bool is_touchpad(const char* dev_path);
bool is_mouse(const char* dev_path);
bool is_keyboard(const char* dev_path);

bool is_touchpad_from_capabilities(const evdev::Capabilities& caps);
bool is_mouse_from_capabilities(const evdev::Capabilities& caps);
bool is_keyboard_from_capabilities(const evdev::Capabilities& caps);

std::string find_first_touchpad();
std::string find_first_mouse();
std::string find_first_keyboard();

void record_events_multi(const std::vector<RecordTarget>& targets,
                         std::ostream& event_out);

#endif  // EVRP_INPUTDEVICE_H

