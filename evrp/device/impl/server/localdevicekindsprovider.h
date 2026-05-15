#pragma once

#include <vector>

#include "evrp/device/api/devicekindsprovider.h"

namespace evrp::device::server {

class LocalDeviceKindsProvider final : public api::IDeviceKindsProvider {
 public:
  LocalDeviceKindsProvider() = default;

  std::vector<evrp::sdk::DeviceKind> kinds() override;
};

}
