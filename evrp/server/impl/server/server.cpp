#include "evrp/server/impl/server/server.h"

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <cerrno>
#include <cstring>

#include <memory>
#include <string>

#include <unistd.h>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/isetting.h"
#include "evrp/server/impl/server/grpc/grpcevrpservice.h"

namespace evrp::server {

Server::Server(const ISetting& settings)
    : listenAddress_(settings.get<std::string>(
          evrp::sdk::kDeviceServerListenAddress, {})),
      ioContext_(),
      workGuard_(asio::make_work_guard(ioContext_.get_executor())),
      localEvrp_(),
      postedEvrp_(localEvrp_, ioContext_),
      worker_([this]() { ioContext_.run(); }),
      evrpService_(std::make_unique<GrpcEvrpService>(&postedEvrp_)) {}

Server::~Server() {
  evrpService_.reset();
  postedEvrp_.shutdown();
  workGuard_.reset();
  ioContext_.stop();
  if (worker_.joinable()) {
    worker_.join();
  }
}

int Server::run() {
  if (listenAddress_.empty()) {
    logError("evrp-server: listen address missing from settings (key '{}')",
             evrp::sdk::kDeviceServerListenAddress);
    return 1;
  }

  grpc::EnableDefaultHealthCheckService(true);

  if (listenAddress_.starts_with("unix:")) {
    const std::string sockPath = listenAddress_.substr(5);
    if (!sockPath.empty()) {
      if (unlink(sockPath.c_str()) != 0 && errno != ENOENT) {
        logWarn("evrp-server: unlink {}: {}", sockPath, std::strerror(errno));
      }
    }
  }

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listenAddress_, grpc::InsecureServerCredentials());
  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
  builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
  builder.AddChannelArgument(
      GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 10000);
  builder.RegisterService(evrpService_.get());
  std::unique_ptr<grpc::Server> grpcServer(builder.BuildAndStart());
  if (!grpcServer) {
    logError("evrp-server: failed to listen on {}", listenAddress_);
    return 1;
  }

  logInfo("evrp-server listening on {}", listenAddress_);
  grpcServer->Wait();
  return 0;
}

std::unique_ptr<IServer> makeServer(const ISetting& settings) {
  return std::make_unique<Server>(settings);
}

}  // namespace evrp::server
