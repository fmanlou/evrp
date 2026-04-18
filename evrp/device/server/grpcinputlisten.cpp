#include "evrp/device/server/grpcinputlisten.h"

#include "evrp/device/internal/tofromproto.h"
#include "evrp/sdk/ioc.h"

#include <google/protobuf/empty.pb.h>

#include <vector>

namespace evrp::device::server {

GrpcInputListenService::GrpcInputListenService(const evrp::Ioc& ioc)
    : listener_(ioc.get<api::IInputListener>()) {}

grpc::Status GrpcInputListenService::StartRecording(
    grpc::ServerContext* /*context*/,
    const v1::StartRecordingRequest* request,
    google::protobuf::Empty* /*response*/) {
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (listener_->isListening()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "recording session already active");
  }
  std::vector<api::DeviceKind> kinds;
  api::fromProto(request->kinds(), &kinds);
  if (!listener_->startListening(kinds)) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "startListening failed (no devices or already listening)");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::WaitForInputEvent(
    grpc::ServerContext* /*context*/,
    const v1::WaitForInputEventRequest* request,
    v1::WaitForInputEventResponse* response) {
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  if (request->timeout_ms() < 0) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "timeout_ms must be >= 0");
  }
  if (!listener_->isListening()) {
    response->set_ready(false);
    return grpc::Status::OK;
  }
  response->set_ready(
      listener_->waitForInputEvent(request->timeout_ms()));
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::ReadInputEvents(
    grpc::ServerContext* /*context*/,
    const google::protobuf::Empty* /*request*/,
    v1::ReadInputEventsResponse* response) {
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  std::vector<api::InputEvent> batch = listener_->readInputEvents();
  for (const api::InputEvent& e : batch) {
    api::toProto(e, response->add_events());
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::StopRecording(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  if (!listener_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "input listener not configured");
  }
  listener_->cancelListening();
  return grpc::Status::OK;
}

}  // namespace evrp::device::server
