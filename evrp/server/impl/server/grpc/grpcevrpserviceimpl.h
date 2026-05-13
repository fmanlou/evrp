#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

class GrpcEvrpServiceImpl final
    : public evrp::v1::server::EvrpService::Service {
 public:
  grpc::Status Record(grpc::ServerContext*,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;

  grpc::Status Replay(grpc::ServerContext*,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;
};

}  // namespace evrp::server
