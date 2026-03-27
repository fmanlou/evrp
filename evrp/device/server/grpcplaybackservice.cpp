#include "evrp/device/server/grpcplaybackservice.h"

#include <google/protobuf/empty.pb.h>

namespace evrp::device::server {

grpc::Status GrpcPlaybackService::UploadRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::UploadRecordingFrame* /*request*/,
    evrp::device::v1::UploadRecordingStatus* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "upload_recording not implemented");
}

grpc::Status GrpcPlaybackService::PlaybackRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "playback_recording not implemented");
}

grpc::Status GrpcPlaybackService::StopPlayback(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "stop_playback not implemented");
}

}  // namespace evrp::device::server
