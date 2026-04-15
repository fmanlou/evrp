#pragma once

#include <vector>

#include "evrp/device/api/inputdevicekindsprovider.h"

namespace evrp::device::server {
namespace api = evrp::device::api;

class LocalInputDeviceKindsProvider final
    : public api::IInputDeviceKindsProvider {
 public:
  LocalInputDeviceKindsProvider() = default;

  std::vector<api::DeviceKind> kinds() override;
};

}  // namespace evrp::device::server
