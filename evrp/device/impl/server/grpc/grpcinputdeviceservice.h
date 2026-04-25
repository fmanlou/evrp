#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/v1/service/service.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::session {
class SessionRegistry;
}

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public v1::InputDeviceService::Service {
 public:
  GrpcInputDeviceService(const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions);

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

 private:
  api::ICursorPosition* cursorPosition_;
  api::IInputDeviceKindsProvider* deviceKindsProvider_;
  evrp::session::SessionRegistry& sessions_;
};

}
