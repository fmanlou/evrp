#pragma once

#include <atomic>
#include <cstdint>
#include <grpcpp/grpcpp.h>
#include <thread>

#include "evrp/device/api/inputlistener.h"
#include "evrp/v1/device/service/inputlisten.grpc.pb.h"

namespace evrp {
class Ioc;
}

namespace evrp::session {
class SessionRegistry;
}

namespace evrp::device::server {

class GrpcInputListenService final
    : public evrp::v1::device::InputListenService::Service {
 public:
  GrpcInputListenService(const evrp::Ioc& ioc, evrp::session::SessionRegistry& sessions);
  ~GrpcInputListenService();

  GrpcInputListenService(const GrpcInputListenService&) = delete;
  GrpcInputListenService& operator=(const GrpcInputListenService&) = delete;
  GrpcInputListenService(GrpcInputListenService&&) = delete;
  GrpcInputListenService& operator=(GrpcInputListenService&&) = delete;

  grpc::Status StartRecording(
      grpc::ServerContext* context,
      const evrp::v1::device::StartRecordingRequest* request,
      google::protobuf::Empty* response) override;

  grpc::Status WaitForInputEvent(
      grpc::ServerContext* context,
      const evrp::v1::device::WaitForInputEventRequest* request,
      evrp::v1::device::WaitForInputEventResponse* response) override;

  grpc::Status ReadInputEvents(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      evrp::v1::device::ReadInputEventsResponse* response) override;

  grpc::Status StopRecording(grpc::ServerContext* context,
                             const google::protobuf::Empty* request,
                             google::protobuf::Empty* response) override;

 private:
  void recordingActivityBump();
  void watchdogLoop();

  api::IInputListener* listener_;
  evrp::session::SessionRegistry& sessions_;
  std::atomic<int64_t> lastRecordingActivityNs_{0};
  std::atomic<bool> watchdogStop_{false};
  std::thread watchdogThread_;
};

}
