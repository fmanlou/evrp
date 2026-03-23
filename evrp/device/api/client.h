#pragma once

#include <functional>
#include <vector>

#include "evrp/device/api/types.h"

namespace evrp {
namespace device {
namespace api {

// 业务端（evrp-app）对设备能力的抽象客户端：与 proto RPC 对齐，不暴露 protobuf / gRPC。
// 具体实现可为 GrpcDeviceClient（链接 gRPC）或测试 Mock；业务只依赖本接口。
class IDeviceClient {
 public:
  virtual ~IDeviceClient() = default;

  virtual ApiResult<void> Ping() = 0;

  virtual ApiResult<void> StartReadInput(const std::vector<DeviceKind>& kinds) = 0;

  virtual ApiResult<void> ReadInputEvents(
      const std::function<void(const InputEvent&)>& on_event) = 0;

  virtual ApiResult<void> StopReadInput() = 0;

  virtual ApiResult<void> UploadRecording(
      const std::function<bool(UploadFrame* frame)>& next_frame_to_send,
      const std::function<void(const RecordingStatus&)>& on_status) = 0;

  virtual ApiResult<PlaybackResponse> PlaybackRecording() = 0;

  virtual ApiResult<void> StopPlayback() = 0;

  virtual ApiResult<CursorAvailability> GetCursorPositionAvailability() = 0;

  virtual ApiResult<CursorPosition> ReadCursorPosition() = 0;
};

}  // namespace api
}  // namespace device
}  // namespace evrp
