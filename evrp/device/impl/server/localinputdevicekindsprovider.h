#pragma once

#include <vector>

#include "evrp/device/api/devicekindsprovider.h"

namespace evrp::device::server {

class LocalInputDeviceKindsProvider final
    : public api::IDeviceKindsProvider {
 public:
  LocalInputDeviceKindsProvider() = default;

  std::vector<evrp::sdk::DeviceKind> kinds() override;
};

}
