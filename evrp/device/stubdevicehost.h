#pragma once

#include <condition_variable>
#include <mutex>

#include "evrp/device/api/host.h"

namespace evrp::device {

// 默认桩：Ping 成功；录制路径实现「StartRecording → ReadInputEvents（阻塞至 StopRecording）」会话模型，
// 不打开 evdev、不推送真实 InputEvent；其余接口仍返回未实现。用于启动 evrp-device 进程并联调 gRPC。
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

  mutable std::mutex recording_mu_;
  std::condition_variable recording_cv_;
  bool recording_session_active_{false};
  bool stop_recording_requested_{false};
};

}  // namespace evrp::device
