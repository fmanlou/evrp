#include "evrp/server/impl/server/server.h"

#include <memory>

#include "evrp/server/impl/server/grpc/grpcevrpservice.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

evrp::server::Server::Server()
    : service_(std::make_unique<evrp::server::GrpcEvrpService>()) {}

evrp::server::Server::~Server() = default;

evrp::server::Server::Server(Server&&) noexcept = default;

evrp::server::Server& evrp::server::Server::operator=(Server&&) noexcept =
    default;

grpc::Service* evrp::server::Server::grpc_service() {
  return service_.get();
}
