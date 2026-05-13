#include "evrp/device/impl/client/remotedevicekindsprovider.h"

#include "evrp/device/impl/client/remoteinputdeviceclient.h"

namespace evrp::device::client {

RemoteDeviceKindsProvider::RemoteDeviceKindsProvider(RemoteInputDeviceClient* device)
    : device_(device) {}

bool RemoteDeviceKindsProvider::queryKinds(std::vector<api::DeviceKind>* out) {
  if (!out || !device_) {
    return false;
  }
  return device_->getCapabilities(out);
}

std::vector<api::DeviceKind> RemoteDeviceKindsProvider::kinds() {
  std::vector<api::DeviceKind> out;
  (void)queryKinds(&out);
  return out;
}

}  // namespace evrp::device::client
