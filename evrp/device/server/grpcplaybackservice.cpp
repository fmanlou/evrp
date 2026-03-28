#include "evrp/device/server/grpcplaybackservice.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::server {

GrpcPlaybackService::GrpcPlaybackService(api::IPlayback& playback)
    : playback_(playback) {}

grpc::Status GrpcPlaybackService::Upload(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::UploadRecordingRequest* request,
    evrp::device::v1::OperationResult* response) {
  api::OperationResult result;
  if (!playback_.upload(api::FromProto(request->events()), &result)) {
    return grpc::Status(
        grpc::StatusCode::INTERNAL,
        result.message.empty() ? "upload failed" : result.message);
  }
  api::ToProto(result, response);
  return grpc::Status::OK;
}

grpc::Status GrpcPlaybackService::Playback(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::OperationResult* response) {
  api::OperationResult result;
  if (!playback_.playback(&result)) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        result.message.empty() ? "playback failed" : result.message);
  }
  api::ToProto(result, response);
  return grpc::Status::OK;
}

grpc::Status GrpcPlaybackService::Stop(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  if (!playback_.stop_playback()) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "stop failed");
  }
  return grpc::Status::OK;
}

}  // namespace evrp::device::server
