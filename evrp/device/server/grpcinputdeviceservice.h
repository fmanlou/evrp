#pragma once

// 设备端 `InputDeviceService` 实现（光标、保活等）；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/v1/service/service.grpc.pb.h"

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public v1::InputDeviceService::Service {
 public:
  GrpcInputDeviceService(api::ICursorPosition* cursorPosition,
                         api::IInputDeviceKindsProvider* deviceKindsProvider);

  GrpcInputDeviceService(const GrpcInputDeviceService&) = delete;
  GrpcInputDeviceService& operator=(const GrpcInputDeviceService&) = delete;

  grpc::Status GetCursorPositionAvailability(
      grpc::ServerContext* context,
      const v1::GetCursorPositionAvailabilityRequest* request,
      v1::GetCursorPositionAvailabilityResponse* response) override;

  grpc::Status ReadCursorPosition(
      grpc::ServerContext* context,
      const v1::ReadCursorPositionRequest* request,
      v1::ReadCursorPositionResponse* response) override;

  grpc::Status GetCapabilities(
      grpc::ServerContext* context,
      const v1::GetCapabilitiesRequest* request,
      v1::GetCapabilitiesResponse* response) override;

  grpc::Status Ping(grpc::ServerContext* context,
                    const v1::PingRequest* request,
                    v1::PingResponse* response) override;

 private:
  api::ICursorPosition* cursorPosition_;
  api::IInputDeviceKindsProvider* deviceKindsProvider_;
};

}  // namespace evrp::device::server
