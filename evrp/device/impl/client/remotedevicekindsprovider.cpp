#include "evrp/device/impl/client/remotedevicekindsprovider.h"

#include "evrp/device/impl/client/remoteinputdeviceclient.h"

namespace evrp::device::client {

RemoteDeviceKindsProvider::RemoteDeviceKindsProvider(RemoteInputDeviceClient* device)
    : device_(device) {}

bool RemoteDeviceKindsProvider::rpcKinds(std::vector<evrp::sdk::DeviceKind>* out) {
  if (!out || !device_) {
    return false;
  }
  return device_->getCapabilities(out);
}

std::vector<evrp::sdk::DeviceKind> RemoteDeviceKindsProvider::kinds() {
  std::vector<evrp::sdk::DeviceKind> out;
  (void)rpcKinds(&out);
  return out;
}

}  // namespace evrp::device::client
