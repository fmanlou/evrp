#pragma once

#include <vector>

#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/server/dispatch/syncdispatchqueue.h"

namespace evrp::device::server {

class DispatchedInputDeviceKindsProvider final
    : public api::IInputDeviceKindsProvider {
 public:
  DispatchedInputDeviceKindsProvider(api::IInputDeviceKindsProvider& inner,
                                     asio::io_context& ioContext);
  ~DispatchedInputDeviceKindsProvider() override;

  DispatchedInputDeviceKindsProvider(const DispatchedInputDeviceKindsProvider&) =
      delete;
  DispatchedInputDeviceKindsProvider& operator=(
      const DispatchedInputDeviceKindsProvider&) = delete;

  void shutdown();

  std::vector<api::DeviceKind> kinds() override;

 private:
  api::IInputDeviceKindsProvider& inner_;
  mutable SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
