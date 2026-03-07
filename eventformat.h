#pragma once

#include "evdev.h"

#include <string>

std::string parse_event_label(const std::string& line);

bool parse_event_line(const std::string& line, long long* out_timestamp_us,
                     unsigned short* out_type, unsigned short* out_code,
                     int* out_value);

std::string event_type_name(unsigned short type);
std::string event_code_name(unsigned short type, unsigned short code);
std::string format_event_line(const std::string& label, const Event& ev);
