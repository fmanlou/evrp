#pragma once

#include <memory>

namespace grpc {
class Service;
}

namespace evrp::server {

class GrpcEvrpService {
 public:
  GrpcEvrpService();
  ~GrpcEvrpService();

  GrpcEvrpService(GrpcEvrpService&&) noexcept;
  GrpcEvrpService& operator=(GrpcEvrpService&&) noexcept;

  GrpcEvrpService(const GrpcEvrpService&) = delete;
  GrpcEvrpService& operator=(const GrpcEvrpService&) = delete;

  grpc::Service* grpc_service();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace evrp::server
