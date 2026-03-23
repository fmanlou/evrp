#include "evrp/device/stub_device_host.h"

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
  return Fail(Unimplemented("StartRecording"));
}

api::ApiResult<void> StubDeviceHost::ReadInputEvents(
    const std::function<void(const api::InputEvent&)>& /*emit*/) {
  return Fail(Unimplemented("ReadInputEvents"));
}

api::ApiResult<void> StubDeviceHost::StopRecording() {
  return Fail(Unimplemented("StopRecording"));
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
