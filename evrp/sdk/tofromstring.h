#pragma once

#include <string>

#include "evrp/sdk/types.h"

namespace evrp::sdk {

DeviceKind toKind(const std::string& label);
void toKind(const std::string& label, DeviceKind* outKind);
std::string toString(DeviceKind kind);

}  // namespace evrp::sdk
