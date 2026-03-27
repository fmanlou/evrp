#include "evrp/device/server/grpcinputdeviceservice.h"

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

grpc::Status GrpcInputDeviceService::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  return grpc::Status::OK;
}

}  // namespace evrp::device::server
