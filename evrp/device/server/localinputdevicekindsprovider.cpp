#include "evrp/device/server/localinputdevicekindsprovider.h"

#include "inputdevice.h"

namespace evrp::device::server {

std::vector<api::DeviceKind> LocalInputDeviceKindsProvider::kinds() {
  static const api::DeviceKind k_order[] = {
      api::DeviceKind::kTouchpad,
      api::DeviceKind::kTouchscreen,
      api::DeviceKind::kMouse,
      api::DeviceKind::kKeyboard,
  };
  std::vector<api::DeviceKind> out;
  for (api::DeviceKind k : k_order) {
    if (!findDevicePath(k).empty()) {
      out.push_back(k);
    }
  }
  return out;
}

}  // namespace evrp::device::server
