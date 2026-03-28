#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

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

// 上传：与 proto UploadRecordingRequest 对齐。
struct UploadFrame {
  std::vector<InputEvent> events;
};

// 统一业务侧错误描述（不暴露 grpc::StatusCode）。
struct ApiError {
  int code = 0;  // 约定 0 表示成功；非 0 为业务/实现自定义。
  std::string message;
  static ApiError success() { return {}; }
  static ApiError make(int c, std::string msg) {
    ApiError e;
    e.code = c;
    e.message = std::move(msg);
    return e;
  }
  bool is_ok() const { return code == 0; }
};

template <typename T>
struct ApiResult {
  ApiError error;
  T value{};
  bool is_ok() const { return error.is_ok(); }
};

template <>
struct ApiResult<void> {
  ApiError error;
  bool is_ok() const { return error.is_ok(); }
};

}  // namespace evrp::device::api
