#pragma once

#include "evrp/device/api/devicekindsprovider.h"

namespace evrp::device::client {

class RemoteInputDeviceClient;

class RemoteDeviceKindsProvider final : public api::IDeviceKindsProvider {
 public:
  explicit RemoteDeviceKindsProvider(RemoteInputDeviceClient* device);

  RemoteDeviceKindsProvider(const RemoteDeviceKindsProvider&) = delete;
  RemoteDeviceKindsProvider& operator=(const RemoteDeviceKindsProvider&) = delete;

  std::vector<api::DeviceKind> kinds() override;
  bool queryKinds(std::vector<api::DeviceKind>* out) override;

 private:
  RemoteInputDeviceClient* device_{};
};

}  // namespace evrp::device::client
