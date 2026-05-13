#pragma once

#include <mutex>

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/playback.h"
#include "evrp/v1/device/service/playback.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::session {
class SessionRegistry;
}

namespace evrp::device::server {

class GrpcPlaybackService final
    : public v1::PlaybackService::Service {
 public:
  GrpcPlaybackService(const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions);

  grpc::Status Upload(
      grpc::ServerContext* context,
      const v1::UploadRecordingRequest* request,
      evrp::sdk::v1::StatusCode* response) override;

  grpc::Status Playback(
      grpc::ServerContext* context,
      const v1::PlaybackRecordingRequest* request,
      evrp::sdk::v1::StatusCode* response) override;

  grpc::Status SubscribePlayback(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      grpc::ServerWriter<v1::PlaybackProgress>* writer) override;

  grpc::Status Stop(grpc::ServerContext* context,
                    const google::protobuf::Empty* request,
                    google::protobuf::Empty* response) override;

 private:
  void markPlaybackStreamFinished();

  api::IPlayback* playback_;
  evrp::session::SessionRegistry& sessions_;

  evrp::CountingSemaphore playbackProgressSem_;

  std::mutex progressMutex_;
  bool subscriberActive_{false};
  bool playbackProgressFinished_{false};
};

}
