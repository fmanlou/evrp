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
#include "evrp/device/impl/server/grpc/grpcsessionservice.h"
#include "evrp/device/impl/server/grpc/grpcinputdeviceservice.h"
#include "evrp/device/impl/server/grpc/grpcinputlisten.h"
#include "evrp/device/impl/server/grpc/grpcplaybackservice.h"
#include "logger.h"

DECLARE_int32(session_lease_ms);

namespace evrp::device::api {

int runDeviceServer(const std::string& listen_address, const evrp::Ioc& ioc) {
  evrp::session::SessionRegistry sessionRegistry(FLAGS_session_lease_ms);
  server::GrpcSessionService session_service(sessionRegistry);
  server::GrpcInputListenService listen_service(ioc, sessionRegistry);
  server::GrpcInputDeviceService device_service(ioc, sessionRegistry);
  server::GrpcPlaybackService playback_service(ioc, sessionRegistry);

  std::thread([&sessionRegistry]() {
    for (;;) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      sessionRegistry.sweepExpiredForLogging();
    }
  }).detach();

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
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
    logError("evrp-device: failed to listen on {}", listen_address);
    return 1;
  }

  logInfo("evrp-device listening on {}", listen_address);
  server->Wait();
  return 0;
}

}
