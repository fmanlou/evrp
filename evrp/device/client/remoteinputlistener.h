#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/service/inputlisten.grpc.pb.h"

namespace evrp::device::client {

class RemoteInputListener final : public api::IInputListener {
 public:
  explicit RemoteInputListener(std::shared_ptr<grpc::Channel> channel);

  ~RemoteInputListener() override;

  RemoteInputListener(const RemoteInputListener&) = delete;
  RemoteInputListener& operator=(const RemoteInputListener&) = delete;

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<evrp::device::v1::InputListenService::Stub> stub_;

  bool listeningActive_{false};
};

}  // namespace evrp::device::client
