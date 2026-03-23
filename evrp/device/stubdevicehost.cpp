#include "evrp/device/stubdevicehost.h"

namespace evrp::device {

namespace {

api::ApiResult<void> fail_result(const api::ApiError& e) {
  api::ApiResult<void> r;
  r.error = e;
  return r;
}

template <typename T>
api::ApiResult<T> fail_result_t(const api::ApiError& e) {
  api::ApiResult<T> r;
  r.error = e;
  return r;
}

}  // namespace

api::ApiError StubDeviceHost::unimplemented(const char* what) {
  return api::ApiError::make(501, what);
}

api::ApiResult<void> StubDeviceHost::ping() {
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::start_recording(
    const std::vector<api::DeviceKind>& /*kinds*/) {
  std::lock_guard<std::mutex> lock(recording_mu_);
  if (recording_session_active_) {
    return fail_result(
        api::ApiError::make(409, "Recording session already active"));
  }
  recording_session_active_ = true;
  stop_recording_requested_ = false;
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::read_input_events(
    const std::function<void(const api::InputEvent&)>& emit) {
  std::unique_lock<std::mutex> lock(recording_mu_);
  if (!recording_session_active_) {
    return fail_result(api::ApiError::make(
        400, "start_recording must be called before read_input_events"));
  }
  api::InputEvent e;
  e.device = api::DeviceKind::kKeyboard;
  e.time_sec = 0;
  e.time_usec = 0;
  e.type = 0;
  e.code = 0;
  e.value = 0;
  emit(e);
  while (!stop_recording_requested_) {
    recording_cv_.wait(lock);
  }
  recording_session_active_ = false;
  stop_recording_requested_ = false;
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::stop_recording() {
  std::lock_guard<std::mutex> lock(recording_mu_);
  stop_recording_requested_ = true;
  recording_cv_.notify_all();
  return api::ApiResult<void>{};
}

api::ApiResult<void> StubDeviceHost::upload_recording(
    const std::function<bool(api::UploadFrame* /*frame*/)>& /*read_next_frame*/,
    const std::function<bool(const api::RecordingStatus&)>& /*emit_status*/) {
  return fail_result(unimplemented("upload_recording"));
}

api::ApiResult<api::PlaybackResponse> StubDeviceHost::playback_recording() {
  return fail_result_t<api::PlaybackResponse>(unimplemented("playback_recording"));
}

api::ApiResult<void> StubDeviceHost::stop_playback() {
  return fail_result(unimplemented("stop_playback"));
}

api::ApiResult<api::CursorAvailability> StubDeviceHost::get_cursor_position_availability() {
  return fail_result_t<api::CursorAvailability>(
      unimplemented("get_cursor_position_availability"));
}

api::ApiResult<api::CursorPosition> StubDeviceHost::read_cursor_position() {
  return fail_result_t<api::CursorPosition>(unimplemented("read_cursor_position"));
}

}  // namespace evrp::device
