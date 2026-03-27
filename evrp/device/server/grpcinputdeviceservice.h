#pragma once

// 设备端 `InputDeviceService` 实现（光标、保活等）；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/v1/service/service.grpc.pb.h"

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  GrpcInputDeviceService() = default;

  grpc::Status GetCursorPositionAvailability(
      grpc::ServerContext* context,
      const evrp::device::v1::GetCursorPositionAvailabilityRequest* request,
      evrp::device::v1::GetCursorPositionAvailabilityResponse* response) override;

  grpc::Status ReadCursorPosition(
      grpc::ServerContext* context,
      const evrp::device::v1::ReadCursorPositionRequest* request,
      evrp::device::v1::ReadCursorPositionResponse* response) override;

  grpc::Status Ping(grpc::ServerContext* context,
                    const evrp::device::v1::PingRequest* request,
                    evrp::device::v1::PingResponse* response) override;
};

}  // namespace evrp::device::server
