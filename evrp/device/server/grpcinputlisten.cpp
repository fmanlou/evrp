#include "evrp/device/server/grpcinputlisten.h"

#include "evrp/device/internal/tofromproto.h"

#include <google/protobuf/empty.pb.h>

#include <vector>

namespace evrp::device::server {

GrpcInputListenService::GrpcInputListenService(api::IInputListener& listener)
    : listener_(listener) {}

grpc::Status GrpcInputListenService::StartRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::StartRecordingRequest* request,
    google::protobuf::Empty* /*response*/) {
  if (listener_.is_listening()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "recording session already active");
  }
  std::vector<api::DeviceKind> kinds;
  api::FromProto(request->kinds(), &kinds);
  if (!listener_.start_listening(kinds)) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "start_listening failed (no devices or already listening)");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::WaitForInputEvent(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::WaitForInputEventRequest* request,
    evrp::device::v1::WaitForInputEventResponse* response) {
  if (request->timeout_ms() < 0) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "timeout_ms must be >= 0");
  }
  if (!listener_.is_listening()) {
    response->set_ready(false);
    return grpc::Status::OK;
  }
  response->set_ready(
      listener_.wait_for_input_event(request->timeout_ms()));
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::ReadInputEvents(
    grpc::ServerContext* /*context*/,
    const google::protobuf::Empty* /*request*/,
    evrp::device::v1::ReadInputEventsResponse* response) {
  std::vector<api::InputEvent> batch = listener_.read_input_events();
  for (const api::InputEvent& e : batch) {
    api::ToProto(e, response->add_events());
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputListenService::StopRecording(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  listener_.cancel_listening();
  return grpc::Status::OK;
}

}  // namespace evrp::device::server
