#pragma once

#include <functional>
#include <vector>

#include "evrp/device/internal/types.h"

namespace evrp::device::api {

// 设备端业务能力抽象：与 device.proto 中 RPC 一一对应，但仅使用 api::* 类型。
// evrp-device 核心逻辑实现本接口；gRPC 层仅作适配，业务代码 include
// 本头文件即可，无需 protobuf/grpc。
class IDeviceHost {
 public:
  virtual ~IDeviceHost() = default;

  virtual ApiResult<void> ping() = 0;

  virtual ApiResult<void> start_recording(const std::vector<DeviceKind>& kinds) = 0;

  // 由适配器在 ReadInputEvents RPC 中调用：阻塞直到会话结束；每产生一条事件调用
  // emit。约定：emit 中不得再调用本 IDeviceHost 的其它接口。
  virtual ApiResult<void> read_input_events(
      const std::function<void(const InputEvent&)>& emit) = 0;

  virtual ApiResult<void> stop_recording() = 0;

  // 上传：适配器从 gRPC 流读出帧并填入 frame（中间帧 data
  // 指针由适配器缓冲，直到下一次 read_next_frame）。 emit_status
  // 向客户端写状态；返回 false 表示流已不可写。
  virtual ApiResult<void> upload_recording(
      const std::function<bool(UploadFrame* frame)>& read_next_frame,
      const std::function<bool(const RecordingStatus&)>& emit_status) = 0;

  virtual ApiResult<PlaybackResponse> playback_recording() = 0;

  virtual ApiResult<void> stop_playback() = 0;

  virtual ApiResult<CursorAvailability> get_cursor_position_availability() = 0;

  virtual ApiResult<CursorPosition> read_cursor_position() = 0;
};

}  // namespace evrp::device::api
