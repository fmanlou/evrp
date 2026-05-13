#pragma once

#include <vector>

#include "evrp/device/api/devicekindsprovider.h"
#include "evrp/sdk/iocontextpostedbase.h"

namespace evrp::device::server {

class PostedInputDeviceKindsProvider final
    : public api::IDeviceKindsProvider,
      private IoContextPostedBase {
 public:
  using IoContextPostedBase::shutdown;

  PostedInputDeviceKindsProvider(api::IDeviceKindsProvider& inner,
                                 asio::io_context& ioContext);
  ~PostedInputDeviceKindsProvider() override;

  PostedInputDeviceKindsProvider(const PostedInputDeviceKindsProvider&) = delete;
  PostedInputDeviceKindsProvider& operator=(
      const PostedInputDeviceKindsProvider&) = delete;

  std::vector<evrp::sdk::DeviceKind> kinds() override;

 private:
  api::IDeviceKindsProvider& inner_;
};

}
