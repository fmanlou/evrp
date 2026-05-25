#include "evrp/device/impl/server/grpcserver.h"

#include <chrono>
#include <memory>
#include <thread>

#include <gflags/gflags.h>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/device/impl/server/grpc/grpcinputdeviceservice.h"
#include "evrp/device/impl/server/grpc/grpcinputlisten.h"
#include "evrp/device/impl/server/grpc/grpcplaybackservice.h"
#include "evrp/sdk/log/grpc/grpclogservice.h"
#include "evrp/device/impl/server/grpc/grpcsessionservice.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/listenaddress.h"
#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/log/logservicetee.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/sdk/setting/isetting.h"

DECLARE_int32(session_lease_ms);

namespace evrp::device::server {

GrpcServer::GrpcServer(const evrp::Ioc& ioc, const ISetting& deviceSettings)
    : listenAddress_(deviceSettings.get<std::string>(
          evrp::sdk::kDeviceServerListenAddress, {})),
      ioc_(ioc),
      deviceSettings_(deviceSettings),
      discoveryTop_(),
      discoveryOverlay_(&discoveryTop_, {&deviceSettings_}),
      discoveryResponder_(createDiscoveryResponder(discoveryOverlay_)) {}

GrpcServer::~GrpcServer() = default;

int GrpcServer::run() {
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
  GrpcSessionService sessionService(sessionRegistry);
  GrpcInputListenService listenService(ioc_, sessionRegistry);
  GrpcInputDeviceService deviceService(ioc_, sessionRegistry);
  GrpcPlaybackService playbackService(ioc_, sessionRegistry);
  GrpcLogService deviceLogStreamService(sessionRegistry);
  logging::ILogService* priorLogService = logService;
  evrp::sdk::LogServiceTee logTee(priorLogService, &deviceLogStreamService);
  if (priorLogService) {
    const logging::LogLevel level = priorLogService->getLevel();
    deviceLogStreamService.logSender()->setLevel(level);
    deviceLogStreamService.logReceiver()->setLevel(level);
  }
  logService = &logTee;

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
  builder.RegisterService(&deviceLogStreamService);
  std::unique_ptr<grpc::Server> grpcServer(builder.BuildAndStart());
  if (!grpcServer) {
    logError("evrp-device: failed to listen on {}", listenAddress_);
    return 1;
  }

  logInfo("evrp-device listening on {}", listenAddress_);
  grpcServer->Wait();
  return 0;
}

}  // namespace evrp::device::server
