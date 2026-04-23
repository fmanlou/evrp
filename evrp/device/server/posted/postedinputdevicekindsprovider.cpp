#include "evrp/device/server/posted/postedinputdevicekindsprovider.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedInputDeviceKindsProvider::PostedInputDeviceKindsProvider(
    api::IInputDeviceKindsProvider& inner, asio::io_context& ioContext)
    : IoContextPostedBase(ioContext), inner_(inner) {}

PostedInputDeviceKindsProvider::~PostedInputDeviceKindsProvider() {
  shutdown();
}

std::vector<api::DeviceKind> PostedInputDeviceKindsProvider::kinds() {
  return syncDispatch_.postSync<std::vector<api::DeviceKind>>(
      [this]() { return inner_.kinds(); });
}

}
