// gRPC 服务端启动：仅在本翻译单元包含 gRPC 头文件。

#include "evrp/device/api/server.h"

#include <iostream>
#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/internal/grpc_input_device_service.h"

namespace evrp::device::api {

int RunDeviceServer(const std::string& listen_address, IDeviceHost& host) {
  internal::GrpcInputDeviceService grpc_service(host);

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&grpc_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "evrp-device: failed to listen on " << listen_address << "\n";
    return 1;
  }

  std::cerr << "evrp-device listening on " << listen_address << "\n";
  server->Wait();
  return 0;
}

}  // namespace evrp::device::api
