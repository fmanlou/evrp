#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace evrp {
namespace device {
namespace api {

// 与 proto DeviceKind 对齐；业务层只使用本枚举，不依赖 protobuf。
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

struct RecordingStatus {
  int32_t code = 0;
  std::string message;
};

struct PlaybackResponse {
  int32_t code = 0;
  std::string message;
};

struct CursorAvailability {
  bool available = false;
};

struct CursorPosition {
  int32_t x = 0;
  int32_t y = 0;
};

// 上传帧：与 proto UploadRecordingFrame oneof 对齐。
struct UploadFrame {
  enum class Kind { kStart, kMiddle, kEnd };
  Kind kind = Kind::kStart;
  const uint8_t* data = nullptr;
  size_t data_len = 0;
  uint32_t checksum = 0;
};

// 统一业务侧错误描述（不暴露 grpc::StatusCode）。
struct ApiError {
  int code = 0;  // 约定 0 表示成功；非 0 为业务/实现自定义。
  std::string message;
  static ApiError Ok() { return {}; }
  static ApiError Make(int c, std::string msg) {
    ApiError e;
    e.code = c;
    e.message = std::move(msg);
    return e;
  }
  bool ok() const { return code == 0; }
};

template <typename T>
struct ApiResult {
  ApiError error;
  T value{};
  bool ok() const { return error.ok(); }
};

template <>
struct ApiResult<void> {
  ApiError error;
  bool ok() const { return error.ok(); }
};

}  // namespace api
}  // namespace device
}  // namespace evrp
