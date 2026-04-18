#pragma once

#include <vector>

#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/server/posted/iocontextpostedbase.h"

namespace evrp::device::server {

class PostedInputDeviceKindsProvider final
    : public api::IInputDeviceKindsProvider,
      private IoContextPostedBase {
 public:
  using IoContextPostedBase::shutdown;

  PostedInputDeviceKindsProvider(api::IInputDeviceKindsProvider& inner,
                                 asio::io_context& ioContext);
  ~PostedInputDeviceKindsProvider() override;

  PostedInputDeviceKindsProvider(const PostedInputDeviceKindsProvider&) = delete;
  PostedInputDeviceKindsProvider& operator=(
      const PostedInputDeviceKindsProvider&) = delete;

  std::vector<api::DeviceKind> kinds() override;

 private:
  api::IInputDeviceKindsProvider& inner_;
};

}  // namespace evrp::device::server
