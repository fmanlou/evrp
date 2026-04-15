#pragma once

// 设备端 `InputDeviceService` 实现（光标、保活等）；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/v1/service/service.grpc.pb.h"

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  GrpcInputDeviceService(evrp::device::api::ICursorPosition& cursor_position,
                         evrp::device::api::IInputDeviceKindsProvider& device_kinds_provider);

  GrpcInputDeviceService(const GrpcInputDeviceService&) = delete;
  GrpcInputDeviceService& operator=(const GrpcInputDeviceService&) = delete;

  grpc::Status GetCursorPositionAvailability(
      grpc::ServerContext* context,
      const evrp::device::v1::GetCursorPositionAvailabilityRequest* request,
      evrp::device::v1::GetCursorPositionAvailabilityResponse* response) override;

  grpc::Status ReadCursorPosition(
      grpc::ServerContext* context,
      const evrp::device::v1::ReadCursorPositionRequest* request,
      evrp::device::v1::ReadCursorPositionResponse* response) override;

  grpc::Status GetCapabilities(
      grpc::ServerContext* context,
      const evrp::device::v1::GetCapabilitiesRequest* request,
      evrp::device::v1::GetCapabilitiesResponse* response) override;

  grpc::Status Ping(grpc::ServerContext* context,
                    const evrp::device::v1::PingRequest* request,
                    evrp::device::v1::PingResponse* response) override;

 private:
  evrp::device::api::ICursorPosition& cursorPosition_;
  evrp::device::api::IInputDeviceKindsProvider& deviceKindsProvider_;
};

}  // namespace evrp::device::server
