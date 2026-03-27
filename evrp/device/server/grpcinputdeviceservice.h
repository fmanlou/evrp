#pragma once

// 设备端 `InputDeviceService` 实现（非监听类 RPC）；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/v1/service.grpc.pb.h"

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  GrpcInputDeviceService() = default;

  grpc::Status UploadRecording(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* stream)
      override;

  grpc::Status PlaybackRecording(
      grpc::ServerContext* context,
      const evrp::device::v1::PlaybackRecordingRequest* request,
      evrp::device::v1::PlaybackRecordingResponse* response) override;

  grpc::Status StopPlayback(grpc::ServerContext* context,
                            const google::protobuf::Empty* request,
                            google::protobuf::Empty* response) override;

  grpc::Status GetCursorPositionAvailability(
      grpc::ServerContext* context,
      const evrp::device::v1::GetCursorPositionAvailabilityRequest* request,
      evrp::device::v1::GetCursorPositionAvailabilityResponse* response) override;

  grpc::Status ReadCursorPosition(
      grpc::ServerContext* context,
      const evrp::device::v1::ReadCursorPositionRequest* request,
      evrp::device::v1::ReadCursorPositionResponse* response) override;

  grpc::Status Ping(grpc::ServerContext* context,
                    const evrp::device::v1::PingRequest* request,
                    evrp::device::v1::PingResponse* response) override;
};

}  // namespace evrp::device::server
