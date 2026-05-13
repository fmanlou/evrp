#pragma once

#include <vector>

#include "evrp/sdk/types.h"

namespace evrp::device::api {

class IDeviceKindsProvider {
 public:
  virtual ~IDeviceKindsProvider() = default;

  virtual std::vector<DeviceKind> kinds() = 0;
};

}
