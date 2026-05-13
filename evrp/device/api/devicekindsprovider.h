#pragma once

#include <vector>

#include "evrp/sdk/types.h"

namespace evrp::device::api {

class IDeviceKindsProvider {
 public:
  virtual ~IDeviceKindsProvider() = default;

  virtual std::vector<evrp::sdk::DeviceKind> kinds() = 0;
};

}
