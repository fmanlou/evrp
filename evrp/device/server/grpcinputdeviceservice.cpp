#include "evrp/device/server/grpcinputdeviceservice.h"

#include "evrp/device/internal/tofromproto.h"
#include "inputdevice.h"

namespace api = evrp::device::api;

namespace evrp::device::server {

grpc::Status GrpcInputDeviceService::GetCursorPositionAvailability(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCursorPositionAvailabilityRequest* /*request*/,
    evrp::device::v1::GetCursorPositionAvailabilityResponse* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "get_cursor_position_availability not implemented");
}

grpc::Status GrpcInputDeviceService::ReadCursorPosition(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::ReadCursorPositionRequest* /*request*/,
    evrp::device::v1::ReadCursorPositionResponse* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "read_cursor_position not implemented");
}

grpc::Status GrpcInputDeviceService::GetCapabilities(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCapabilitiesRequest* /*request*/,
    evrp::device::v1::GetCapabilitiesResponse* response) {
  static const api::DeviceKind k_probe_order[] = {
      api::DeviceKind::kTouchpad,
      api::DeviceKind::kTouchscreen,
      api::DeviceKind::kMouse,
      api::DeviceKind::kKeyboard,
  };
  for (api::DeviceKind k : k_probe_order) {
    if (!findDevicePath(k).empty()) {
      response->add_supported_kinds(api::toProto(k));
    }
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  return grpc::Status::OK;
}

}
