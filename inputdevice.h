#pragma once

#include "evdev/evdev.h"

#include <string>

bool is_touchpad(const char* dev_path);
bool is_mouse(const char* dev_path);
bool is_keyboard(const char* dev_path);

bool is_mouse_from_capabilities(const evdev::Capabilities& caps);
evdev::Event make_event(unsigned short type, unsigned short code, int value);

std::string find_first_touchpad();
std::string find_first_mouse();
std::string find_first_keyboard();

std::string find_device_path(const std::string& label);

