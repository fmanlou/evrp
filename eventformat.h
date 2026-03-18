#pragma once

#include <string>

#include "deviceid.h"
#include "evdev.h"

std::string parse_event_label(const std::string &line);

// Parses event line; out_delta_us is relative interval in microseconds.
bool parse_event_line(const std::string &line, long long *out_delta_us,
                      unsigned short *out_type, unsigned short *out_code,
                      int *out_value);

std::string event_type_name(unsigned short type);
std::string event_code_name(unsigned short type, unsigned short code);
// delta_us: relative interval in microseconds (0 for first event).
std::string format_event_line(DeviceId id, const Event &ev, long long delta_us);
