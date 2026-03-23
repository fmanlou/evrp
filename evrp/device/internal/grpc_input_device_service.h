#pragma once

// 内部实现：仅由 grpc_server_impl.cpp / 本目录 .cpp 包含，业务代码勿 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/host.h"
#include "evrp/device/v1/device.grpc.pb.h"

namespace evrp::device::internal {

class GrpcInputDeviceService final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  explicit GrpcInputDeviceService(api::IDeviceHost& host);

  grpc::Status StartRecording(
      grpc::ServerContext* context,
      const evrp::device::v1::StartRecordingRequest* request,
      google::protobuf::Empty* response) override;

  grpc::Status ReadInputEvents(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      grpc::ServerWriter<evrp::device::v1::InputEvent>* writer) override;

  grpc::Status StopRecording(grpc::ServerContext* context,
                             const google::protobuf::Empty* request,
                             google::protobuf::Empty* response) override;

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

 private:
  api::IDeviceHost& host_;
};

}  // namespace evrp::device::internal
