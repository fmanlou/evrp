#include "evrp/device/impl/server/grpc/grpcinputdeviceservice.h"

#include "evrp/sdk/tofromproto.h"
#include "evrp/sdk/sessioncheck.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/log/logger.h"

namespace evrp::device::server {

GrpcInputDeviceService::GrpcInputDeviceService(
    const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions)
    : cursorPosition_(ioc.get<api::ICursorPosition>()),
      deviceKindsProvider_(ioc.get<api::IDeviceKindsProvider>()),
      sessions_(sessions) {}

grpc::Status GrpcInputDeviceService::GetCursorPositionAvailability(
    grpc::ServerContext* context,
    const evrp::v1::device::GetCursorPositionAvailabilityRequest* ,
    evrp::v1::device::GetCursorPositionAvailabilityResponse* response) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!cursorPosition_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "cursor position not configured");
  }
  response->set_available(cursorPosition_->getCursorPositionAvailability());
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadCursorPosition(
    grpc::ServerContext* context,
    const evrp::v1::device::ReadCursorPositionRequest* ,
    evrp::v1::device::ReadCursorPositionResponse* response) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!cursorPosition_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "cursor position not configured");
  }
  if (!cursorPosition_->getCursorPositionAvailability()) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "cursor position not available");
  }
  int x = 0;
  int y = 0;
  if (!cursorPosition_->readCursorPosition(&x, &y)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "read_cursor_position failed");
  }
  response->set_x(x);
  response->set_y(y);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::GetCapabilities(
    grpc::ServerContext* context,
    const evrp::v1::device::GetCapabilitiesRequest* ,
    evrp::v1::device::GetCapabilitiesResponse* response) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!deviceKindsProvider_) {
    logError(
        "evrp-device: GetCapabilities: IDeviceKindsProvider is null "
        "(IOC misconfiguration; not the same as \"no input hardware\")");
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "device kinds provider not configured");
  }
  for (evrp::sdk::DeviceKind k : deviceKindsProvider_->kinds()) {
    response->add_supported_kinds(evrp::sdk::toProto(k));
  }
  return grpc::Status::OK;
}

}
