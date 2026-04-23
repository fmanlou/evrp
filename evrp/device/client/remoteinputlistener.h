#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/service/inputlisten.grpc.pb.h"

#include <string>

namespace evrp::device::client {

class RemoteInputListener final : public api::IInputListener {
 public:
  RemoteInputListener(std::shared_ptr<grpc::Channel> channel,
                      std::string deviceSessionId);

  ~RemoteInputListener() override;

  RemoteInputListener(const RemoteInputListener&) = delete;
  RemoteInputListener& operator=(const RemoteInputListener&) = delete;

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  std::unique_ptr<v1::InputListenService::Stub> stub_;
  std::string deviceSessionId_;

  bool listeningActive_{false};
};

}  // namespace evrp::device::client
