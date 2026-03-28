#include "evrp/device/server/grpcplaybackservice.h"

#include <google/protobuf/empty.pb.h>

namespace evrp::device::server {

grpc::Status GrpcPlaybackService::Upload(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::UploadRecordingRequest* /*request*/,
    evrp::device::v1::UploadRecordingStatus* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "upload not implemented");
}

grpc::Status GrpcPlaybackService::Playback(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "playback not implemented");
}

grpc::Status GrpcPlaybackService::Stop(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                      "stop not implemented");
}

}  // namespace evrp::device::server
