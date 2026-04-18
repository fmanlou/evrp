#include "evrp/device/server/dispatch/dispatchedinputdevicekindsprovider.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

DispatchedInputDeviceKindsProvider::DispatchedInputDeviceKindsProvider(
    api::IInputDeviceKindsProvider& inner, asio::io_context& ioContext)
    : inner_(inner), syncDispatch_(ioContext) {}

DispatchedInputDeviceKindsProvider::~DispatchedInputDeviceKindsProvider() {
  shutdown();
}

void DispatchedInputDeviceKindsProvider::shutdown() {
  syncDispatch_.shutdown();
}

std::vector<api::DeviceKind> DispatchedInputDeviceKindsProvider::kinds() {
  return syncDispatch_.postSync<std::vector<api::DeviceKind>>(
      [this]() { return inner_.kinds(); });
}

}  // namespace evrp::device::server
