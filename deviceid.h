#pragma once

#include <string>

enum class DeviceId { Keyboard, Mouse, Touchpad, Touchscreen, Unknown };

DeviceId deviceIdFromLabel(const std::string &label);
std::string deviceLabel(DeviceId id);
