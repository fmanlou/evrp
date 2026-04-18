#pragma once

#include <vector>

#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/sdk/syncdispatchqueue.h"

namespace evrp::device::server {

class PostedInputDeviceKindsProvider final
    : public api::IInputDeviceKindsProvider {
 public:
  PostedInputDeviceKindsProvider(api::IInputDeviceKindsProvider& inner,
                                 asio::io_context& ioContext);
  ~PostedInputDeviceKindsProvider() override;

  PostedInputDeviceKindsProvider(const PostedInputDeviceKindsProvider&) = delete;
  PostedInputDeviceKindsProvider& operator=(
      const PostedInputDeviceKindsProvider&) = delete;

  void shutdown();

  std::vector<api::DeviceKind> kinds() override;

 private:
  api::IInputDeviceKindsProvider& inner_;
  mutable SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
