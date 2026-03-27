#pragma once

// 设备端 gRPC 服务实现：由 server/grpcserverimpl.cpp 使用；业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/service.grpc.pb.h"

namespace evrp::device::server {

class GrpcInputDeviceService final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  explicit GrpcInputDeviceService(api::IInputListener& listener);

  grpc::Status StartRecording(
      grpc::ServerContext* context,
      const evrp::device::v1::StartRecordingRequest* request,
      google::protobuf::Empty* response) override;

  grpc::Status WaitForInputEvent(
      grpc::ServerContext* context,
      const evrp::device::v1::WaitForInputEventRequest* request,
      evrp::device::v1::WaitForInputEventResponse* response) override;

  grpc::Status ReadInputEvents(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      evrp::device::v1::ReadInputEventsResponse* response) override;

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
  api::IInputListener& listener_;
};

}  // namespace evrp::device::server
