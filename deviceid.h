#pragma once

#include <string>

enum class DeviceId { Keyboard, Mouse, Touchpad, Unknown };

DeviceId device_id_from_label(const std::string& label);
std::string device_label(DeviceId id);
