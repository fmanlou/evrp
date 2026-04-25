#include "evrp/device/impl/server/grpc/grpcplaybackservice.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/tofromproto.h"
#include "evrp/sdk/devicesessioncheck.h"
#include "evrp/sdk/devicesessionregistry.h"
#include "evrp/sdk/ioc.h"

namespace evrp::device::server {

GrpcPlaybackService::GrpcPlaybackService(const evrp::Ioc& ioc,
                                         DeviceSessionRegistry& sessions)
    : playback_(ioc.get<api::IPlayback>()), sessions_(sessions) {}

void GrpcPlaybackService::markPlaybackStreamFinished() {
  std::lock_guard<std::mutex> lock(progressMutex_);
  playbackProgressFinished_ = true;
}

grpc::Status GrpcPlaybackService::Upload(
    grpc::ServerContext* context,
    const v1::UploadRecordingRequest* request,
    v1::OperationResult* response) {
  if (grpc::Status st = requireDeviceBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!playback_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "playback not configured");
  }
  api::OperationResult result;
  std::vector<api::InputEvent> events = api::fromProto(request->events());
  if (!playback_->upload(events, &result)) {
    return grpc::Status(
        grpc::StatusCode::INTERNAL,
        result.message.empty() ? "upload failed" : result.message);
  }
  api::toProto(result, response);
  return grpc::Status::OK;
}

grpc::Status GrpcPlaybackService::Playback(
    grpc::ServerContext* context,
    const v1::PlaybackRecordingRequest* ,
    v1::OperationResult* response) {
  if (grpc::Status st = requireDeviceBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  while (playbackProgressSem_.tryAcquire()) {
  }
  {
    std::lock_guard<std::mutex> lock(progressMutex_);
    playbackProgressFinished_ = false;
  }

  api::OperationResult result;
  const bool ok = playback_->playback(&result, &playbackProgressSem_);

  markPlaybackStreamFinished();
  playbackProgressSem_.release();

  if (!ok) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        result.message.empty() ? "playback failed" : result.message);
  }
  api::toProto(result, response);
  return grpc::Status::OK;
}

grpc::Status GrpcPlaybackService::SubscribePlayback(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    grpc::ServerWriter<v1::PlaybackProgress>* writer) {
  if (grpc::Status st = requireDeviceBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!playback_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "playback not configured");
  }
  {
    std::lock_guard<std::mutex> lock(progressMutex_);
    if (subscriberActive_) {
      return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED,
                          "subscribe_playback already active");
    }
    subscriberActive_ = true;
    playbackProgressFinished_ = false;
  }
  while (playbackProgressSem_.tryAcquire()) {
  }

  grpc::Status status = grpc::Status::OK;
  for (;;) {
    if (context->IsCancelled()) {
      status = grpc::Status(grpc::StatusCode::CANCELLED, "cancelled");
      break;
    }

    playbackProgressSem_.acquire();

    bool finished = false;
    {
      std::lock_guard<std::mutex> lock(progressMutex_);
      finished = playbackProgressFinished_;
    }
    if (finished) {
      break;
    }

    v1::PlaybackProgress msg;
    msg.set_event_index(playback_->playbackIndex());
    if (!writer->Write(msg)) {
      status = grpc::Status(grpc::StatusCode::UNKNOWN, "progress write failed");
      break;
    }
  }

  {
    std::lock_guard<std::mutex> lock(progressMutex_);
    subscriberActive_ = false;
  }
  return status;
}

grpc::Status GrpcPlaybackService::Stop(
    grpc::ServerContext* context, const google::protobuf::Empty* ,
    google::protobuf::Empty* ) {
  if (grpc::Status st = requireDeviceBusinessSession(context, sessions_); !st.ok()) {
    return st;
  }
  if (!playback_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "playback not configured");
  }
  if (!playback_->stopPlayback()) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "stop failed");
  }
  return grpc::Status::OK;
}

}
