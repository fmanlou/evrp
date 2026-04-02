#pragma once

#include <string>

#include "deviceid.h"
#include "evdev.h"

bool isTouchpad(const char *dev_path);
bool isTouchscreen(const char *dev_path);
bool isMouse(const char *dev_path);
bool isKeyboard(const char *dev_path);

bool isMouseFromCapabilities(const Capabilities &caps);
Event makeEvent(unsigned short type, unsigned short code, int value);

std::string findFirstTouchpad();
std::string findFirstTouchscreen();
std::string findFirstMouse();
std::string findFirstKeyboard();

std::string findDevicePath(DeviceId id);
