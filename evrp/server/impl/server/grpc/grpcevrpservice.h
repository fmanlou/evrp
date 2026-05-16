#pragma once

#include <grpcpp/grpcpp.h>

#include <google/protobuf/empty.pb.h>

#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::session {
class SessionRegistry;
}

namespace evrp::server {

class Evrp;

class GrpcEvrpService final
    : public evrp::v1::server::EvrpService::Service {
 public:
  GrpcEvrpService(Evrp* evrp, evrp::session::SessionRegistry& clientSessions);

  grpc::Status Record(grpc::ServerContext* context,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;

  grpc::Status Replay(grpc::ServerContext* context,
                      const google::protobuf::Struct* request,
                      evrp::v1::sdk::StatusCode* response) override;

  grpc::Status IsRecording(grpc::ServerContext*,
                           const google::protobuf::Empty* request,
                           evrp::v1::server::BoolPayload* response) override;

  grpc::Status IsReplaying(grpc::ServerContext*,
                           const google::protobuf::Empty* request,
                           evrp::v1::server::BoolPayload* response) override;

  grpc::Status StopRecording(grpc::ServerContext*,
                             const google::protobuf::Empty* request,
                             evrp::v1::sdk::StatusCode* response) override;

  grpc::Status StopReplay(grpc::ServerContext*,
                          const google::protobuf::Empty* request,
                          evrp::v1::sdk::StatusCode* response) override;

 private:
  grpc::Status requireEvrpClientSession(grpc::ServerContext* context);

  Evrp* evrp_;
  evrp::session::SessionRegistry& clientSessions_;
};

}  // namespace evrp::server
