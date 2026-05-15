#include "evrp/device/api/server.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/device/internal/discovery/discoveryresponder.h"
#include "evrp/device/impl/server/deviceruntime.h"
#include "evrp/device/impl/server/grpc/grpcinputdeviceservice.h"
#include "evrp/sdk/ioc.h"
#include "evrp/device/impl/server/grpc/grpcinputlisten.h"
#include "evrp/device/impl/server/grpc/grpcplaybackservice.h"
#include "evrp/device/impl/server/grpc/grpcsessionservice.h"
#include "evrp/sdk/listenaddress.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/sdk/setting/isetting.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/setting/overlaysetting.h"

DECLARE_int32(session_lease_ms);

namespace evrp::device::api {

namespace {

class GrpcServer {
 public:
  GrpcServer(const evrp::Ioc& ioc, const ISetting& deviceSettings)
      : listenAddress_(deviceSettings.get<std::string>(
            evrp::sdk::kDeviceServerListenAddress, {})),
        ioc_(ioc),
        deviceSettings_(deviceSettings),
        discoveryTop_(),
        discoveryOverlay_(&discoveryTop_, {&deviceSettings_}),
        discoveryResponder_(
            evrp::device::server::createDiscoveryResponder(discoveryOverlay_)) {}

  int run() {
    std::uint16_t grpcPort = 0;
    if (evrp::sdk::parseListenPort(listenAddress_, &grpcPort)) {
      discoveryTop_.insert(evrp::sdk::kDeviceDiscoverySettingGrpcListenPort,
                           static_cast<int>(grpcPort));
      discoveryResponder_->start();
    } else {
      logError("evrp-device: could not parse gRPC listen port from {}",
               listenAddress_);
    }

    evrp::session::SessionRegistry sessionRegistry(FLAGS_session_lease_ms);
    evrp::device::server::GrpcSessionService sessionService(sessionRegistry);
    evrp::device::server::GrpcInputListenService listenService(ioc_,
                                                               sessionRegistry);
    evrp::device::server::GrpcInputDeviceService deviceService(ioc_,
                                                               sessionRegistry);
    evrp::device::server::GrpcPlaybackService playbackService(ioc_,
                                                              sessionRegistry);

    std::thread([&sessionRegistry]() {
      for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sessionRegistry.sweepExpiredForLogging();
      }
    }).detach();

    grpc::EnableDefaultHealthCheckService(true);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(listenAddress_, grpc::InsecureServerCredentials());
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.AddChannelArgument(
        GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 10000);
    builder.RegisterService(&sessionService);
    builder.RegisterService(&listenService);
    builder.RegisterService(&deviceService);
    builder.RegisterService(&playbackService);
    std::unique_ptr<grpc::Server> grpcServer(builder.BuildAndStart());
    if (!grpcServer) {
      logError("evrp-device: failed to listen on {}", listenAddress_);
      return 1;
    }

    logInfo("evrp-device listening on {}", listenAddress_);
    grpcServer->Wait();
    return 0;
  }

 private:
  std::string listenAddress_;
  const evrp::Ioc& ioc_;
  const ISetting& deviceSettings_;
  MemorySetting discoveryTop_;
  OverlaySetting discoveryOverlay_;
  std::unique_ptr<evrp::device::server::IDiscoveryResponder> discoveryResponder_;
};

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
  GrpcServer grpcServer_;
};

}  // namespace

std::unique_ptr<IServer> makeServer(const ISetting& device_settings) {
  return std::make_unique<Server>(device_settings);
}

}  // namespace evrp::device::api
