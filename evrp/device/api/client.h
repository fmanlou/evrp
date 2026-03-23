#pragma once

#include <functional>
#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

// 业务端（evrp-app）对设备能力的抽象客户端：与 proto RPC 对齐，不暴露 protobuf / gRPC。
// 具体实现可为 GrpcDeviceClient（链接 gRPC）或测试 Mock；业务只依赖本接口。
class IDeviceClient {
 public:
  virtual ~IDeviceClient() = default;

  virtual ApiResult<void> ping() = 0;

  virtual ApiResult<void> start_recording(const std::vector<DeviceKind>& kinds) = 0;

  virtual ApiResult<void> read_input_events(
      const std::function<void(const InputEvent&)>& on_event) = 0;

  virtual ApiResult<void> stop_recording() = 0;

  virtual ApiResult<void> upload_recording(
      const std::function<bool(UploadFrame* frame)>& next_frame_to_send,
      const std::function<void(const RecordingStatus&)>& on_status) = 0;

  virtual ApiResult<PlaybackResponse> playback_recording() = 0;

  virtual ApiResult<void> stop_playback() = 0;

  virtual ApiResult<CursorAvailability> get_cursor_position_availability() = 0;

  virtual ApiResult<CursorPosition> read_cursor_position() = 0;
};

}  // namespace evrp::device::api
