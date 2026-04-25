#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/device/impl/server/devicesessionregistry.h"
#include "evrp/device/v1/service/session.grpc.pb.h"

namespace evrp::device::server {

class GrpcDeviceSessionService final
    : public v1::DeviceSessionService::Service {
 public:
  explicit GrpcDeviceSessionService(DeviceSessionRegistry& registry);

  grpc::Status Connect(grpc::ServerContext* context,
                       const google::protobuf::Empty* request,
                       v1::ConnectResponse* response) override;

  grpc::Status Heartbeat(grpc::ServerContext* context,
                         const google::protobuf::Empty* request,
                         google::protobuf::Empty* response) override;

  grpc::Status Disconnect(grpc::ServerContext* context,
                          const google::protobuf::Empty* request,
                          google::protobuf::Empty* response) override;

 private:
  DeviceSessionRegistry& registry_;
};

}
