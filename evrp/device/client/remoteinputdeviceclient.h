#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/inputdeviceclient.h"
#include "evrp/device/v1/service/service.grpc.pb.h"

namespace evrp::device::client {

class RemoteInputDeviceClient final : public api::IInputDeviceClient {
 public:
  explicit RemoteInputDeviceClient(std::shared_ptr<grpc::Channel> channel);

  RemoteInputDeviceClient(const RemoteInputDeviceClient&) = delete;
  RemoteInputDeviceClient& operator=(const RemoteInputDeviceClient&) = delete;

  bool getCapabilities(std::vector<api::DeviceKind>* out) override;

  bool getCursorPositionAvailability(bool* available) override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<v1::InputDeviceService::Stub> stub_;
};

}  // namespace evrp::device::client
