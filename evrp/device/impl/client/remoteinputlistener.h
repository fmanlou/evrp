#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/v1/device/service/inputlisten.grpc.pb.h"

#include <string>

namespace evrp::device::client {

class RemoteInputListener final : public api::IInputListener {
 public:
  RemoteInputListener(std::shared_ptr<grpc::Channel> channel,
                      std::string deviceSessionId);

  ~RemoteInputListener() override;

  RemoteInputListener(const RemoteInputListener&) = delete;
  RemoteInputListener& operator=(const RemoteInputListener&) = delete;

  bool startListening(const std::vector<evrp::sdk::DeviceKind>& kinds) override;

  std::vector<evrp::sdk::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  std::unique_ptr<evrp::v1::device::InputListenService::Stub> stub_;
  std::string deviceSessionId_;

  bool listeningActive_{false};
};

}
