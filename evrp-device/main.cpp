// evrp-device：设备端 gRPC 服务（InputDeviceService）。
// 业务实现通过 api::IDeviceHost，不直接使用 proto/grpc。
// 用法: evrp-device [--listen ADDRESS]
// 默认 ADDRESS = 127.0.0.1:50051

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "evrp/device/stub_device_host.h"
#include "grpc/grpc_input_device_service.h"

namespace {

const char* kDefaultListen = "127.0.0.1:50051";

void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [--listen ADDRESS]\n"
            << "  ADDRESS  gRPC listen address (default " << kDefaultListen << ")\n";
}

std::string ParseListen(int argc, char** argv) {
  std::string addr = kDefaultListen;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--listen") == 0 && i + 1 < argc) {
      addr = argv[++i];
    } else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
      PrintUsage(argv[0]);
      std::exit(0);
    } else {
      std::cerr << "Unknown argument: " << argv[i] << "\n";
      PrintUsage(argv[0]);
      std::exit(1);
    }
  }
  return addr;
}

}  // namespace

int main(int argc, char** argv) {
  const std::string listen_addr = ParseListen(argc, argv);

  evrp::device::StubDeviceHost host_impl;
  evrp::device::GrpcInputDeviceService grpc_service(host_impl);

  grpc::EnableDefaultHealthCheckService(true);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(listen_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&grpc_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "evrp-device: failed to listen on " << listen_addr << "\n";
    return 1;
  }

  std::cerr << "evrp-device listening on " << listen_addr << "\n";
  server->Wait();
  return 0;
}
