#pragma once

#include <string>

std::string parse_event_label(const std::string& line);

bool parse_event_line(const std::string& line, long long* out_timestamp_us,
                     unsigned short* out_type, unsigned short* out_code,
                     int* out_value);
