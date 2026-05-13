#pragma once

#include <memory>

namespace grpc {
class Service;
}

namespace evrp::server {

class HostControlGrpcService {
 public:
  HostControlGrpcService();
  ~HostControlGrpcService();

  HostControlGrpcService(HostControlGrpcService&&) noexcept;
  HostControlGrpcService& operator=(HostControlGrpcService&&) noexcept;

  HostControlGrpcService(const HostControlGrpcService&) = delete;
  HostControlGrpcService& operator=(const HostControlGrpcService&) = delete;

  grpc::Service* grpc_service();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace evrp::server
