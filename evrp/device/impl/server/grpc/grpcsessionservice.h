#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/sdk/sessionregistry.h"
#include "evrp/v1/sdk/services/session.grpc.pb.h"

namespace evrp::device::server {

class GrpcSessionService final
    : public ::evrp::v1::sdk::SessionService::Service {
 public:
  explicit GrpcSessionService(evrp::session::SessionRegistry& registry);

  grpc::Status Connect(grpc::ServerContext* context,
                       const google::protobuf::Empty* request,
                       ::evrp::v1::sdk::ConnectResponse* response) override;

  grpc::Status Heartbeat(grpc::ServerContext* context,
                         const google::protobuf::Empty* request,
                         google::protobuf::Empty* response) override;

  grpc::Status Disconnect(grpc::ServerContext* context,
                            const google::protobuf::Empty* request,
                            google::protobuf::Empty* response) override;

 private:
  evrp::session::SessionRegistry& registry_;
};

}
