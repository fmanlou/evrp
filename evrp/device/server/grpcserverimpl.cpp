#include "evrp/device/api/server.h"

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/server/grpcinputdeviceservice.h"
#include "evrp/device/server/grpcinputlisten.h"
#include "evrp/device/server/grpcplaybackservice.h"
#include "logger.h"

namespace evrp::device::api {

int runDeviceServer(const std::string& listen_address, const evrp::Ioc& ioc) {
  server::GrpcInputListenService listen_service(ioc);
  server::GrpcInputDeviceService device_service(ioc);
  server::GrpcPlaybackService playback_service(ioc);

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&listen_service);
  builder.RegisterService(&device_service);
  builder.RegisterService(&playback_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    logError("evrp-device: failed to listen on " + listen_address);
    return 1;
  }

  logInfo("evrp-device listening on " + listen_address);
  server->Wait();
  return 0;
}

}  // namespace evrp::device::api
