#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/inputdeviceclient.h"
#include "evrp/device/v1/service/service.grpc.pb.h"

#include <string>

namespace evrp::device::client {

class RemoteInputDeviceClient final : public api::IInputDeviceClient {
 public:
  RemoteInputDeviceClient(std::shared_ptr<grpc::Channel> channel,
                          std::string deviceSessionId);

  RemoteInputDeviceClient(const RemoteInputDeviceClient&) = delete;
  RemoteInputDeviceClient& operator=(const RemoteInputDeviceClient&) = delete;

  bool getCapabilities(std::vector<api::DeviceKind>* out) override;

  bool getCursorPositionAvailability(bool* available) override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<v1::InputDeviceService::Stub> stub_;
  std::string deviceSessionId_;
};

}  // namespace evrp::device::client
