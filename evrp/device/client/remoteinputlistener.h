#pragma once

#include <memory>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/v1/device.grpc.pb.h"

namespace evrp::device::client {

// 通过 gRPC 将 `IInputListener` 映射到 `InputDeviceService`：`StartRecording` /
// `WaitForInputEvent` / `ReadInputEvents` / `StopRecording`。约定：虚函数在同一线程调用。
class RemoteInputListener final : public api::IInputListener {
 public:
  explicit RemoteInputListener(std::shared_ptr<grpc::Channel> channel);

  ~RemoteInputListener() override;

  RemoteInputListener(const RemoteInputListener&) = delete;
  RemoteInputListener& operator=(const RemoteInputListener&) = delete;

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> read_input_events() override;

  bool wait_for_input_event(int timeout_ms) override;

  void cancel_listening() override;

  bool is_listening() const override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<evrp::device::v1::InputDeviceService::Stub> stub_;

  bool listening_active_{false};
};

}  // namespace evrp::device::client
