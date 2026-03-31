// gRPC 服务端启动（evrp/device/server）。仅在本翻译单元包含 gRPC 头文件。

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

int run_device_server(const std::string& listen_address,
                      IInputListener& input_listener,
                      IPlayback& playback) {
  ::evrp::device::server::GrpcInputListenService listen_service(input_listener);
  ::evrp::device::server::GrpcInputDeviceService device_service;
  ::evrp::device::server::GrpcPlaybackService playback_service(playback);

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&listen_service);
  builder.RegisterService(&device_service);
  builder.RegisterService(&playback_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    log_error("evrp-device: failed to listen on " + listen_address);
    return 1;
  }

  log_info("evrp-device listening on " + listen_address);
  server->Wait();
  return 0;
}

}  // namespace evrp::device::api
