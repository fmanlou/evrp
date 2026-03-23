#include "device_service.h"

#include <grpcpp/support/status.h>

namespace evrp {
namespace device {

namespace {

grpc::Status Unimplemented(const char* what) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, what);
}

}  // namespace

grpc::Status InputDeviceServiceImpl::StartReadInput(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::StartReadInputRequest* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return Unimplemented("StartReadInput");
}

grpc::Status InputDeviceServiceImpl::ReadInputEvents(
    grpc::ServerContext* /*context*/,
    const google::protobuf::Empty* /*request*/,
    grpc::ServerWriter<evrp::device::v1::InputEvent>* /*writer*/) {
  return Unimplemented("ReadInputEvents");
}

grpc::Status InputDeviceServiceImpl::StopReadInput(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return Unimplemented("StopReadInput");
}

grpc::Status InputDeviceServiceImpl::UploadRecording(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* /*stream*/) {
  return Unimplemented("UploadRecording");
}

grpc::Status InputDeviceServiceImpl::PlaybackRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* /*response*/) {
  return Unimplemented("PlaybackRecording");
}

grpc::Status InputDeviceServiceImpl::StopPlayback(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return Unimplemented("StopPlayback");
}

grpc::Status InputDeviceServiceImpl::GetCursorPositionAvailability(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCursorPositionAvailabilityRequest* /*request*/,
    evrp::device::v1::GetCursorPositionAvailabilityResponse* /*response*/) {
  return Unimplemented("GetCursorPositionAvailability");
}

grpc::Status InputDeviceServiceImpl::ReadCursorPosition(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::ReadCursorPositionRequest* /*request*/,
    evrp::device::v1::ReadCursorPositionResponse* /*response*/) {
  return Unimplemented("ReadCursorPosition");
}

grpc::Status InputDeviceServiceImpl::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  return grpc::Status::OK;
}

}  // namespace device
}  // namespace evrp
