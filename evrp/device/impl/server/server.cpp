#include "evrp/device/api/server.h"

#include <memory>

#include "evrp/device/impl/server/deviceruntime.h"
#include "evrp/device/impl/server/grpcserver.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/setting/isetting.h"

namespace evrp::device::api {

namespace {

class Server final : public IServer {
 public:
  explicit Server(const ISetting& deviceSettings)
      : ioc_(),
        deviceRuntime_(),
        grpcServer_(ioc_, deviceSettings) {
    deviceRuntime_.registerWith(ioc_);
  }

  int run() override { return grpcServer_.run(); }

 private:
  evrp::Ioc ioc_;
  evrp::device::server::DeviceRuntime deviceRuntime_;
  evrp::device::server::GrpcServer grpcServer_;
};

}  // namespace

std::unique_ptr<IServer> makeServer(const ISetting& device_settings) {
  return std::make_unique<Server>(device_settings);
}

}  // namespace evrp::device::api
