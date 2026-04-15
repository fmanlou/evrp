#pragma once

// 设备端 `InputListenService` 实现；由 grpcserverimpl.cpp 注册。业务代码勿直接 include。

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/service/inputlisten.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::device::server {

class GrpcInputListenService final
    : public v1::InputListenService::Service {
 public:
  explicit GrpcInputListenService(const evrp::Ioc& ioc);

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
  api::IInputListener* listener_;
};

}  // namespace evrp::device::server
