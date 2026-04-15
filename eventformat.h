#pragma once

#include <string>

#include "evrp/device/api/types.h"
#include "evdev.h"

std::string parseEventLabel(const std::string &line);

// Parses event line; out_delta_us is relative interval in microseconds.
bool parseEventLine(const std::string &line, long long *out_delta_us,
                      unsigned short *out_type, unsigned short *out_code,
                      int *out_value);

// Parses [leading] or [trailing] delta_sec.delta_usec line. Returns true on success.
bool parseLeadingLine(const std::string &line, long long *out_delta_us);
bool parseTrailingLine(const std::string &line, long long *out_delta_us);

std::string eventTypeName(unsigned short type);
std::string eventCodeName(unsigned short type, unsigned short code);
// delta_us: relative interval in microseconds (0 for first event).
std::string formatEventLine(evrp::device::api::DeviceKind device,
                            const Event &ev, long long delta_us);

std::string formatLeadingLine(long long delta_us);
std::string formatTrailingLine(long long delta_us);
