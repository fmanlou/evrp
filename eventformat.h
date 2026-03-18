#pragma once

#include <string>

#include "deviceid.h"
#include "evdev.h"

std::string parse_event_label(const std::string &line);

// Parses event line; out_delta_us is relative interval in microseconds.
bool parse_event_line(const std::string &line, long long *out_delta_us,
                      unsigned short *out_type, unsigned short *out_code,
                      int *out_value);

// Parses [leading] or [trailing] delta_sec.delta_usec line. Returns true on success.
bool parse_leading_line(const std::string &line, long long *out_delta_us);
bool parse_trailing_line(const std::string &line, long long *out_delta_us);

std::string event_type_name(unsigned short type);
std::string event_code_name(unsigned short type, unsigned short code);
// delta_us: relative interval in microseconds (0 for first event).
std::string format_event_line(DeviceId id, const Event &ev, long long delta_us);

std::string format_leading_line(long long delta_us);
std::string format_trailing_line(long long delta_us);
