#pragma once

#include <memory>

#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

class GrpcEvrpServices {
 public:
  GrpcEvrpServices();
  ~GrpcEvrpServices();

  GrpcEvrpServices(GrpcEvrpServices&&) noexcept;
  GrpcEvrpServices& operator=(GrpcEvrpServices&&) noexcept;

  GrpcEvrpServices(const GrpcEvrpServices&) = delete;
  GrpcEvrpServices& operator=(const GrpcEvrpServices&) = delete;

  grpc::Service* grpc_service();

 private:
  std::unique_ptr<evrp::v1::server::EvrpService::Service> service_;
};

}  // namespace evrp::server
