#pragma once

#include <vector>

#include "evrp/device/api/inputdevicekindsprovider.h"

namespace evrp::device::server {

class LocalInputDeviceKindsProvider final
    : public evrp::device::api::IInputDeviceKindsProvider {
 public:
  LocalInputDeviceKindsProvider() = default;

  std::vector<evrp::device::api::DeviceKind> kinds() override;
};

}  // namespace evrp::device::server
