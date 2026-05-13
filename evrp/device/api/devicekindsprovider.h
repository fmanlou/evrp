#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IDeviceKindsProvider {
 public:
  virtual ~IDeviceKindsProvider() = default;

  virtual std::vector<DeviceKind> kinds() = 0;
};

}
