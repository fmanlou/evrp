#pragma once

// 设备端 `PlaybackService` 实现；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/v1/service/playback.grpc.pb.h"

namespace evrp::device::server {

class GrpcPlaybackService final
    : public evrp::device::v1::PlaybackService::Service {
 public:
  GrpcPlaybackService() = default;

  grpc::Status UploadRecording(
      grpc::ServerContext* context,
      const evrp::device::v1::UploadRecordingFrame* request,
      evrp::device::v1::UploadRecordingStatus* response) override;

  grpc::Status PlaybackRecording(
      grpc::ServerContext* context,
      const evrp::device::v1::PlaybackRecordingRequest* request,
      evrp::device::v1::PlaybackRecordingResponse* response) override;

  grpc::Status StopPlayback(grpc::ServerContext* context,
                            const google::protobuf::Empty* request,
                            google::protobuf::Empty* response) override;
};

}  // namespace evrp::device::server
