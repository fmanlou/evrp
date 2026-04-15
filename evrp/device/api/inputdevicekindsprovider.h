#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IInputDeviceKindsProvider {
 public:
  virtual ~IInputDeviceKindsProvider() = default;

  virtual std::vector<DeviceKind> kinds() = 0;
};

}  // namespace evrp::device::api
