#ifndef EVRP_INPUT_DEVICE_H
#define EVRP_INPUT_DEVICE_H

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

std::string find_first_touchpad();
std::string find_first_mouse();
std::string find_first_keyboard();

void record_events_multi(const std::vector<RecordTarget>& targets);

#endif  // EVRP_INPUT_DEVICE_H
