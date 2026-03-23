#pragma once

#include "evrp/device/api/host.h"

namespace evrp::device {

// 默认桩：仅 Ping 成功，其余返回「未实现」；用于启动 evrp-device 进程。
class StubDeviceHost final : public api::IDeviceHost {
 public:
  api::ApiResult<void> Ping() override;

  api::ApiResult<void> StartRecording(const std::vector<api::DeviceKind>& kinds) override;

  api::ApiResult<void> ReadInputEvents(
      const std::function<void(const api::InputEvent&)>& emit) override;

  api::ApiResult<void> StopRecording() override;

  api::ApiResult<void> UploadRecording(
      const std::function<bool(api::UploadFrame* frame)>& read_next_frame,
      const std::function<bool(const api::RecordingStatus&)>& emit_status) override;

  api::ApiResult<api::PlaybackResponse> PlaybackRecording() override;

  api::ApiResult<void> StopPlayback() override;

  api::ApiResult<api::CursorAvailability> GetCursorPositionAvailability() override;

  api::ApiResult<api::CursorPosition> ReadCursorPosition() override;

 private:
  static api::ApiError Unimplemented(const char* what);
};

}  // namespace evrp::device
