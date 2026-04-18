#include "evrp/device/server/posted/postedinputdevicekindsprovider.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedInputDeviceKindsProvider::PostedInputDeviceKindsProvider(
    api::IInputDeviceKindsProvider& inner, asio::io_context& ioContext)
    : inner_(inner), syncDispatch_(ioContext) {}

PostedInputDeviceKindsProvider::~PostedInputDeviceKindsProvider() {
  shutdown();
}

void PostedInputDeviceKindsProvider::shutdown() {
  syncDispatch_.shutdown();
}

std::vector<api::DeviceKind> PostedInputDeviceKindsProvider::kinds() {
  return syncDispatch_.postSync<std::vector<api::DeviceKind>>(
      [this]() { return inner_.kinds(); });
}

}  // namespace evrp::device::server
