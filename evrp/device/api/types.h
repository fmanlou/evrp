#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace evrp::device::api {

// 与 proto DeviceKind 对齐。互转见 evrp/device/internal/tofromproto.h（ToProto / FromProto）。
enum class DeviceKind {
  kUnspecified = 0,
  kTouchpad = 1,
  kTouchscreen = 2,
  kMouse = 3,
  kKeyboard = 4,
};

// 与 proto InputEvent 对齐。
struct InputEvent {
  DeviceKind device = DeviceKind::kUnspecified;
  int64_t time_sec = 0;
  int64_t time_usec = 0;
  uint32_t type = 0;
  uint32_t code = 0;
  int32_t value = 0;
};

// 与 proto `UploadRecordingRequest` 对齐；事件列表互转见 `evrp/device/internal/tofromproto.h`。
struct RecordingUpload {
  std::vector<InputEvent> events;
};

// 通用应答体：`code` / `message`；与 proto `UploadRecordingStatus`、`PlaybackRecordingResponse`
// 等 unary 应答字段一致。
struct OperationResult {
  int32_t code = 0;
  std::string message;
};

}  // namespace evrp::device::api
