#pragma once

#include <string>

#include "evrp/device/api/types.h"
#include "evrp/sdk/evdev.h"

std::string parseEventLabel(const std::string &line);

bool parseEventLine(const std::string &line, long long *out_deltaUs,
                      unsigned short *out_type, unsigned short *out_code,
                      int *out_value);

bool parseLeadingLine(const std::string &line, long long *out_deltaUs);
bool parseTrailingLine(const std::string &line, long long *out_deltaUs);

std::string eventTypeName(unsigned short type);
std::string eventCodeName(unsigned short type, unsigned short code);

std::string formatEventLine(evrp::device::api::DeviceKind device,
                            const Event &ev, long long deltaUs);

std::string formatLeadingLine(long long deltaUs);
std::string formatTrailingLine(long long deltaUs);
