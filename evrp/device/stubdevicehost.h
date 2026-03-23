#pragma once

#include <condition_variable>
#include <mutex>

#include "evrp/device/api/host.h"

namespace evrp::device {

// 默认桩：ping 成功；录制路径实现「start_recording → read_input_events（阻塞至
// stop_recording）」会话模型， 不打开 evdev、不推送真实
// InputEvent；其余接口仍返回未实现。用于启动 evrp-device 进程并联调 gRPC。
class StubDeviceHost final : public api::IDeviceHost {
 public:
  api::ApiResult<void> ping() override;

  api::ApiResult<void> start_recording(
      const std::vector<api::DeviceKind>& kinds) override;

  api::ApiResult<void> read_input_events(
      const std::function<void(const api::InputEvent&)>& emit) override;

  api::ApiResult<void> stop_recording() override;

  api::ApiResult<void> upload_recording(
      const std::function<bool(api::UploadFrame* frame)>& read_next_frame,
      const std::function<bool(const api::RecordingStatus&)>& emit_status)
      override;

  api::ApiResult<api::PlaybackResponse> playback_recording() override;

  api::ApiResult<void> stop_playback() override;

  api::ApiResult<api::CursorAvailability> get_cursor_position_availability()
      override;

  api::ApiResult<api::CursorPosition> read_cursor_position() override;

 private:
  static api::ApiError unimplemented(const char* what);

  mutable std::mutex recording_mu_;
  std::condition_variable recording_cv_;
  bool recording_session_active_{false};
  bool stop_recording_requested_{false};
};

}  // namespace evrp::device
