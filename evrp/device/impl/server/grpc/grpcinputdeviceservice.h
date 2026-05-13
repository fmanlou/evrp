#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/devicekindsprovider.h"
#include "evrp/v1/device/service/service.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::session {
class SessionRegistry;
}

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public evrp::v1::device::InputDeviceService::Service {
 public:
  GrpcInputDeviceService(const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions);

  GrpcInputDeviceService(const GrpcInputDeviceService&) = delete;
  GrpcInputDeviceService& operator=(const GrpcInputDeviceService&) = delete;

  grpc::Status GetCursorPositionAvailability(
      grpc::ServerContext* context,
      const evrp::v1::device::GetCursorPositionAvailabilityRequest* request,
      evrp::v1::device::GetCursorPositionAvailabilityResponse* response) override;

  grpc::Status ReadCursorPosition(
      grpc::ServerContext* context,
      const evrp::v1::device::ReadCursorPositionRequest* request,
      evrp::v1::device::ReadCursorPositionResponse* response) override;

  grpc::Status GetCapabilities(
      grpc::ServerContext* context,
      const evrp::v1::device::GetCapabilitiesRequest* request,
      evrp::v1::device::GetCapabilitiesResponse* response) override;

 private:
  api::ICursorPosition* cursorPosition_;
  api::IDeviceKindsProvider* deviceKindsProvider_;
  evrp::session::SessionRegistry& sessions_;
};

}
