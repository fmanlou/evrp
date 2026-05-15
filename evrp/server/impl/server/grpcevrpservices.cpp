#include "evrp/server/impl/server/grpcevrpservices.h"

#include <memory>

#include "evrp/server/impl/server/grpc/grpcevrpservice.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

GrpcEvrpServices::GrpcEvrpServices()
    : service_(std::make_unique<GrpcEvrpService>()) {}

GrpcEvrpServices::~GrpcEvrpServices() = default;

GrpcEvrpServices::GrpcEvrpServices(GrpcEvrpServices&&) noexcept = default;

GrpcEvrpServices& GrpcEvrpServices::operator=(GrpcEvrpServices&&) noexcept =
    default;

grpc::Service* GrpcEvrpServices::grpc_service() {
  return service_.get();
}

}  // namespace evrp::server
