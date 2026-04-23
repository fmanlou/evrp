#pragma once

// 设备端 `InputListenService`（unary）。业务代码勿直接 include。

#include <atomic>
#include <cstdint>
#include <grpcpp/grpcpp.h>
#include <thread>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/service/inputlisten.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::device::server {

class DeviceSessionRegistry;

class GrpcInputListenService final
    : public v1::InputListenService::Service {
 public:
  GrpcInputListenService(const evrp::Ioc& ioc, DeviceSessionRegistry& sessions);
  ~GrpcInputListenService();

  GrpcInputListenService(const GrpcInputListenService&) = delete;
  GrpcInputListenService& operator=(const GrpcInputListenService&) = delete;
  GrpcInputListenService(GrpcInputListenService&&) = delete;
  GrpcInputListenService& operator=(GrpcInputListenService&&) = delete;

  grpc::Status StartRecording(
      grpc::ServerContext* context,
      const v1::StartRecordingRequest* request,
      google::protobuf::Empty* response) override;

  grpc::Status WaitForInputEvent(
      grpc::ServerContext* context,
      const v1::WaitForInputEventRequest* request,
      v1::WaitForInputEventResponse* response) override;

  grpc::Status ReadInputEvents(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      v1::ReadInputEventsResponse* response) override;

  grpc::Status StopRecording(grpc::ServerContext* context,
                             const google::protobuf::Empty* request,
                             google::protobuf::Empty* response) override;

 private:
  void recordingActivityBump();
  void watchdogLoop();

  api::IInputListener* listener_;
  DeviceSessionRegistry& sessions_;
  std::atomic<int64_t> lastRecordingActivityNs_{0};
  std::atomic<bool> watchdogStop_{false};
  std::thread watchdogThread_;
};

}  // namespace evrp::device::server
