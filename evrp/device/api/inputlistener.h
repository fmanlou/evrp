#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

// 进程内输入监听会话（非 gRPC）：start_listening → read_input_events → cancel_listening。
// 与 IDeviceHost 的「StartRecording / ReadInputEvents(emit) / StopRecording」语义相近但 API 不同。
class IInputListener {
 public:
  virtual ~IInputListener() = default;

  virtual bool start_listening(const std::vector<DeviceKind>& kinds) = 0;

  virtual std::vector<InputEvent> read_input_events() = 0;

  virtual void cancel_listening() = 0;
};

}  // namespace evrp::device::api
