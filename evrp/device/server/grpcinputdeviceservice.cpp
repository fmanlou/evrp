#include "evrp/device/server/grpcinputdeviceservice.h"

#include "evrp/device/internal/tofromproto.h"

namespace api = evrp::device::api;

namespace evrp::device::server {

GrpcInputDeviceService::GrpcInputDeviceService(
    api::ICursorPosition& cursor_position,
    api::IInputDeviceKindsProvider& device_kinds_provider)
    : cursor_position_(cursor_position),
      device_kinds_provider_(device_kinds_provider) {}

grpc::Status GrpcInputDeviceService::GetCursorPositionAvailability(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCursorPositionAvailabilityRequest* /*request*/,
    evrp::device::v1::GetCursorPositionAvailabilityResponse* response) {
  response->set_available(cursor_position_.getCursorPositionAvailability());
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadCursorPosition(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::ReadCursorPositionRequest* /*request*/,
    evrp::device::v1::ReadCursorPositionResponse* response) {
  if (!cursor_position_.getCursorPositionAvailability()) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "cursor position not available");
  }
  int x = 0;
  int y = 0;
  if (!cursor_position_.readCursorPosition(&x, &y)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "read_cursor_position failed");
  }
  response->set_x(x);
  response->set_y(y);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::GetCapabilities(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCapabilitiesRequest* /*request*/,
    evrp::device::v1::GetCapabilitiesResponse* response) {
  for (api::DeviceKind k : device_kinds_provider_.kinds()) {
    response->add_supported_kinds(api::toProto(k));
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  return grpc::Status::OK;
}

}
