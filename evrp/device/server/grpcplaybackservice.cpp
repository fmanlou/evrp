#include "evrp/device/server/grpcplaybackservice.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::server {

namespace api = evrp::device::api;

GrpcPlaybackService::GrpcPlaybackService(api::IPlayback& playback)
    : playback_(playback) {}

void GrpcPlaybackService::mark_playback_stream_finished() {
  std::lock_guard<std::mutex> lock(prog_mu_);
  prog_playback_finished_ = true;
}

grpc::Status GrpcPlaybackService::Upload(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::UploadRecordingRequest* request,
    evrp::device::v1::OperationResult* response) {
  api::OperationResult result;
  std::vector<api::InputEvent> events = api::FromProto(request->events());
  if (!playback_.upload(events, &result)) {
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
  while (playback_progress_sem_.try_acquire()) {
  }
  {
    std::lock_guard<std::mutex> lock(prog_mu_);
    prog_playback_finished_ = false;
  }

  api::OperationResult result;
  const bool ok = playback_.playback(&result, &playback_progress_sem_);

  mark_playback_stream_finished();
  playback_progress_sem_.release();

  if (!ok) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        result.message.empty() ? "playback failed" : result.message);
  }
  api::ToProto(result, response);
  return grpc::Status::OK;
}

grpc::Status GrpcPlaybackService::SubscribePlayback(
    grpc::ServerContext* context,
    const google::protobuf::Empty* /*request*/,
    grpc::ServerWriter<evrp::device::v1::PlaybackProgress>* writer) {
  {
    std::lock_guard<std::mutex> lock(prog_mu_);
    if (subscriber_active_) {
      return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED,
                          "subscribe_playback already active");
    }
    subscriber_active_ = true;
    prog_playback_finished_ = false;
  }
  while (playback_progress_sem_.try_acquire()) {
  }

  grpc::Status status = grpc::Status::OK;
  for (;;) {
    if (context->IsCancelled()) {
      status = grpc::Status(grpc::StatusCode::CANCELLED, "cancelled");
      break;
    }

    playback_progress_sem_.acquire();

    bool finished = false;
    {
      std::lock_guard<std::mutex> lock(prog_mu_);
      finished = prog_playback_finished_;
    }
    if (finished) {
      break;
    }

    evrp::device::v1::PlaybackProgress msg;
    msg.set_event_index(playback_.playback_index());
    if (!writer->Write(msg)) {
      status = grpc::Status(grpc::StatusCode::UNKNOWN, "progress write failed");
      break;
    }
  }

  {
    std::lock_guard<std::mutex> lock(prog_mu_);
    subscriber_active_ = false;
  }
  return status;
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
