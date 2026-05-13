#include "evrp/device/impl/client/remotedevicekindsprovider.h"

#include "evrp/device/impl/client/remoteinputdeviceclient.h"

namespace evrp::device::client {

RemoteDeviceKindsProvider::RemoteDeviceKindsProvider(RemoteInputDeviceClient* device)
    : device_(device) {}

bool RemoteDeviceKindsProvider::rpcKinds(std::vector<api::DeviceKind>* out) {
  if (!out || !device_) {
    return false;
  }
  return device_->getCapabilities(out);
}

std::vector<api::DeviceKind> RemoteDeviceKindsProvider::kinds() {
  std::vector<api::DeviceKind> out;
  (void)rpcKinds(&out);
  return out;
}

}  // namespace evrp::device::client
