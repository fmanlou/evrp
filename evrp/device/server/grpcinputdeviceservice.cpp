#include "evrp/device/server/grpcinputdeviceservice.h"

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
