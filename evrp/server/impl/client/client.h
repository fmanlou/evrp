#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>

namespace evrp::server {

class Evrp;

std::shared_ptr<grpc::Channel> createGrpcChannel();

std::unique_ptr<Evrp> makeEvrp(std::shared_ptr<grpc::Channel> channel);

}  // namespace evrp::server
