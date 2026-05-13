#pragma once

#include "evrp/device/api/devicekindsprovider.h"

namespace evrp::device::client {

class RemoteInputDeviceClient;

class RemoteDeviceKindsProvider final : public api::IDeviceKindsProvider {
 public:
  explicit RemoteDeviceKindsProvider(RemoteInputDeviceClient* device);

  RemoteDeviceKindsProvider(const RemoteDeviceKindsProvider&) = delete;
  RemoteDeviceKindsProvider& operator=(const RemoteDeviceKindsProvider&) = delete;

  std::vector<evrp::sdk::DeviceKind> kinds() override;

  /** Fills kinds via GetCapabilities RPC; false on RPC/null device (integration / diagnostics). */
  bool rpcKinds(std::vector<evrp::sdk::DeviceKind>* out);

 private:
  RemoteInputDeviceClient* device_{};
};

}  // namespace evrp::device::client
