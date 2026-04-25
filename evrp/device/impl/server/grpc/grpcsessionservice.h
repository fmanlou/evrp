#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/sdk/sessionregistry.h"
#include "evrp/sdk/v1/services/session.grpc.pb.h"

namespace evrp::device::server {

class GrpcSessionService final
    : public ::evrp::sdk::v1::SessionService::Service {
 public:
  explicit GrpcSessionService(evrp::session::SessionRegistry& registry);

  grpc::Status Connect(grpc::ServerContext* context,
                       const google::protobuf::Empty* request,
                       ::evrp::sdk::v1::ConnectResponse* response) override;

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
