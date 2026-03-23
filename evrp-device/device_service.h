#pragma once

#include <grpcpp/grpcpp.h>

#include "evrp/device/v1/device.grpc.pb.h"

namespace evrp {
namespace device {

// InputDeviceService 的首版实现：Ping 可用，其余 RPC 返回 UNIMPLEMENTED（后续按 EVRP_DEVICE_DEVELOPMENT_PLAN 填充）。
class InputDeviceServiceImpl final
    : public evrp::device::v1::InputDeviceService::Service {
 public:
  grpc::Status StartReadInput(
      grpc::ServerContext* context,
      const evrp::device::v1::StartReadInputRequest* request,
      google::protobuf::Empty* response) override;

  grpc::Status ReadInputEvents(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      grpc::ServerWriter<evrp::device::v1::InputEvent>* writer) override;

  grpc::Status StopReadInput(grpc::ServerContext* context,
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
};

}  // namespace device
}  // namespace evrp
