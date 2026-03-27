#include "evrp/device/server/grpcinputdeviceservice.h"

#include "evrp/device/internal/tofromproto.h"

#include <google/protobuf/empty.pb.h>

#include <vector>

namespace evrp::device::server {
namespace {

void DrainUploadStream(
    grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* stream) {
  evrp::device::v1::UploadRecordingFrame msg;
  while (stream->Read(&msg)) {
  }
}

}  // namespace

GrpcInputDeviceService::GrpcInputDeviceService(api::IInputListener& listener)
    : listener_(listener) {}

grpc::Status GrpcInputDeviceService::StartRecording(
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

grpc::Status GrpcInputDeviceService::WaitForInputEvent(
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

grpc::Status GrpcInputDeviceService::ReadInputEvents(
    grpc::ServerContext* /*context*/,
    const google::protobuf::Empty* /*request*/,
    evrp::device::v1::ReadInputEventsResponse* response) {
  std::vector<api::InputEvent> batch = listener_.read_input_events();
  for (const api::InputEvent& e : batch) {
    api::ToProto(e, response->add_events());
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::StopRecording(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  listener_.cancel_listening();
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::UploadRecording(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* stream) {
  DrainUploadStream(stream);
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "upload_recording not implemented");
}

grpc::Status GrpcInputDeviceService::PlaybackRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "playback_recording not implemented");
}

grpc::Status GrpcInputDeviceService::StopPlayback(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "stop_playback not implemented");
}

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
