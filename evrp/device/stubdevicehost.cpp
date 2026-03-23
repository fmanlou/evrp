#include "evrp/device/stubdevicehost.h"

namespace evrp::device {

namespace {

api::ApiResult<void> Fail(const api::ApiError& e) {
  api::ApiResult<void> r;
  r.error = e;
  return r;
}

template <typename T>
api::ApiResult<T> FailT(const api::ApiError& e) {
  api::ApiResult<T> r;
  r.error = e;
  return r;
}

}  // namespace

api::ApiError StubDeviceHost::Unimplemented(const char* what) {
  return api::ApiError::Make(501, what);
}

api::ApiResult<void> StubDeviceHost::Ping() {
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::StartRecording(
    const std::vector<api::DeviceKind>& /*kinds*/) {
  std::lock_guard<std::mutex> lock(recording_mu_);
  if (recording_session_active_) {
    return Fail(api::ApiError::Make(409, "Recording session already active"));
  }
  recording_session_active_ = true;
  stop_recording_requested_ = false;
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::ReadInputEvents(
    const std::function<void(const api::InputEvent&)>& /*emit*/) {
  std::unique_lock<std::mutex> lock(recording_mu_);
  if (!recording_session_active_) {
    return Fail(api::ApiError::Make(
        400, "StartRecording must be called before ReadInputEvents"));
  }
  while (!stop_recording_requested_) {
    recording_cv_.wait(lock);
  }
  recording_session_active_ = false;
  stop_recording_requested_ = false;
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::StopRecording() {
  std::lock_guard<std::mutex> lock(recording_mu_);
  stop_recording_requested_ = true;
  recording_cv_.notify_all();
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::UploadRecording(
    const std::function<bool(api::UploadFrame* /*frame*/)>& /*read_next_frame*/,
    const std::function<bool(const api::RecordingStatus&)>& /*emit_status*/) {
  return Fail(Unimplemented("UploadRecording"));
}

api::ApiResult<api::PlaybackResponse> StubDeviceHost::PlaybackRecording() {
  return FailT<api::PlaybackResponse>(Unimplemented("PlaybackRecording"));
}

api::ApiResult<void> StubDeviceHost::StopPlayback() {
  return Fail(Unimplemented("StopPlayback"));
}

api::ApiResult<api::CursorAvailability> StubDeviceHost::GetCursorPositionAvailability() {
  return FailT<api::CursorAvailability>(
      Unimplemented("GetCursorPositionAvailability"));
}

api::ApiResult<api::CursorPosition> StubDeviceHost::ReadCursorPosition() {
  return FailT<api::CursorPosition>(Unimplemented("ReadCursorPosition"));
}

}  // namespace evrp::device
