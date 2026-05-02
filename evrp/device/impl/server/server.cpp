#include "evrp/device/api/server.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/sdk/sessionregistry.h"
#include "evrp/device/internal/discovery/discoveryresponder.h"
#include "evrp/device/impl/server/grpc/grpcsessionservice.h"
#include "evrp/device/impl/server/grpc/grpcinputdeviceservice.h"
#include "evrp/device/impl/server/grpc/grpcinputlisten.h"
#include "evrp/device/impl/server/grpc/grpcplaybackservice.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/isetting.h"

DECLARE_int32(session_lease_ms);

namespace evrp::device::api {

namespace {

class ServerImpl final : public IServer {
 public:
  ServerImpl(std::string listen_address,
             const evrp::Ioc& ioc,
             const ISetting& device_settings)
      : listen_address_(std::move(listen_address)),
        ioc_(ioc),
        device_settings_(device_settings),
        discoveryResponder_(
            evrp::device::server::createDiscoveryResponder(device_settings)) {}

  int run() override {
    std::uint16_t grpc_port = 0;
    if (evrp::device::server::parseListenPort(listen_address_, &grpc_port)) {
      discoveryResponder_->start(grpc_port);
    } else {
      logError("evrp-device: could not parse gRPC listen port from {}", listen_address_);
    }

    evrp::session::SessionRegistry sessionRegistry(FLAGS_session_lease_ms);
    server::GrpcSessionService session_service(sessionRegistry);
    server::GrpcInputListenService listen_service(ioc_, sessionRegistry);
    server::GrpcInputDeviceService device_service(ioc_, sessionRegistry);
    server::GrpcPlaybackService playback_service(ioc_, sessionRegistry);

    std::thread([&sessionRegistry]() {
      for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        sessionRegistry.sweepExpiredForLogging();
      }
    }).detach();

    grpc::EnableDefaultHealthCheckService(true);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(listen_address_, grpc::InsecureServerCredentials());
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.AddChannelArgument(
        GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 10000);
    builder.RegisterService(&session_service);
    builder.RegisterService(&listen_service);
    builder.RegisterService(&device_service);
    builder.RegisterService(&playback_service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    if (!server) {
      logError("evrp-device: failed to listen on {}", listen_address_);
      return 1;
    }

    logInfo("evrp-device listening on {}", listen_address_);
    server->Wait();
    return 0;
  }

 private:
  std::string listen_address_;
  const evrp::Ioc& ioc_;
  const ISetting& device_settings_;
  std::unique_ptr<evrp::device::server::IDiscoveryResponder> discoveryResponder_;
};

}  // namespace

std::unique_ptr<IServer> makeServer(const std::string& listen_address,
                                    const evrp::Ioc& ioc,
                                    const ISetting& device_settings) {
  return std::make_unique<ServerImpl>(listen_address, ioc, device_settings);
}

}  // namespace evrp::device::api
