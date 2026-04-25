#include "evrp/device/impl/server/grpc/grpcsessionservice.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/sdk/sessioncheck.h"

namespace evrp::device::server {

GrpcSessionService::GrpcSessionService(
    evrp::session::SessionRegistry& registry)
    : registry_(registry) {}

grpc::Status GrpcSessionService::Connect(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    ::evrp::sdk::v1::ConnectResponse* response) {
  const std::string id = registry_.connect(context->peer());
  response->set_session_id(id);
  response->set_lease_timeout_ms(registry_.leaseTimeoutMs());
  return grpc::Status::OK;
}

grpc::Status GrpcSessionService::Heartbeat(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  const auto sid = evrp::session::sessionIdFromContext(*context);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry_.heartbeat(sv, context->peer());
}

grpc::Status GrpcSessionService::Disconnect(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  const auto sid = evrp::session::sessionIdFromContext(*context);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry_.disconnect(sv, context->peer());
}

}
