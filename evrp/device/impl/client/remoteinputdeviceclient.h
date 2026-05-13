#pragma once

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "evrp/sdk/types.h"
#include "evrp/v1/device/service/service.grpc.pb.h"

namespace evrp::device::client {

class RemoteInputDeviceClient final {
 public:
  RemoteInputDeviceClient(std::shared_ptr<grpc::Channel> channel,
                          std::string deviceSessionId);

  RemoteInputDeviceClient(const RemoteInputDeviceClient&) = delete;
  RemoteInputDeviceClient& operator=(const RemoteInputDeviceClient&) = delete;

  bool getCapabilities(std::vector<evrp::sdk::DeviceKind>* out);

  bool getCursorPositionAvailability(bool* available);

  bool readCursorPosition(int* outX, int* outY);

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<v1::InputDeviceService::Stub> stub_;
  std::string deviceSessionId_;
};

}
