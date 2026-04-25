#include "evrp/device/impl/server/grpc/grpcdevicesessionservice.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/sdk/devicesessioncheck.h"

namespace evrp::device::server {

GrpcDeviceSessionService::GrpcDeviceSessionService(DeviceSessionRegistry& registry)
    : registry_(registry) {}

grpc::Status GrpcDeviceSessionService::Connect(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    v1::ConnectResponse* response) {
  const std::string id = registry_.connect(context->peer());
  response->set_session_id(id);
  response->set_lease_timeout_ms(registry_.leaseTimeoutMs());
  return grpc::Status::OK;
}

grpc::Status GrpcDeviceSessionService::Heartbeat(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  const auto sid = deviceSessionIdFromContext(*context);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry_.heartbeat(sv, context->peer());
}

grpc::Status GrpcDeviceSessionService::Disconnect(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  const auto sid = deviceSessionIdFromContext(*context);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry_.disconnect(sv, context->peer());
}

}
