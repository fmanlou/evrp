#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

class Evrp;

class GrpcEvrpService final
    : public evrp::v1::server::EvrpService::Service {
 public:
  explicit GrpcEvrpService(Evrp* evrp);

  grpc::Status Record(grpc::ServerContext*,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;

  grpc::Status Replay(grpc::ServerContext*,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;

 private:
  Evrp* evrp_;
};

}  // namespace evrp::server
