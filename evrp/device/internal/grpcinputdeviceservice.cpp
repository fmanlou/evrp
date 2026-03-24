#include "evrp/device/internal/grpcinputdeviceservice.h"

#include <google/protobuf/empty.pb.h>

#include <chrono>
#include <thread>
#include <vector>

namespace evrp::device::internal {
namespace {

grpc::Status ToGrpc(const api::ApiError& e) {
  if (e.is_ok()) return grpc::Status::OK;
  if (e.code == 501) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, e.message);
  }
  return grpc::Status(grpc::StatusCode::UNKNOWN, e.message);
}

evrp::device::v1::DeviceKind ToProtoEnum(api::DeviceKind k) {
  switch (k) {
    case api::DeviceKind::kTouchpad:
      return evrp::device::v1::DEVICE_KIND_TOUCHPAD;
    case api::DeviceKind::kTouchscreen:
      return evrp::device::v1::DEVICE_KIND_TOUCHSCREEN;
    case api::DeviceKind::kMouse:
      return evrp::device::v1::DEVICE_KIND_MOUSE;
    case api::DeviceKind::kKeyboard:
      return evrp::device::v1::DEVICE_KIND_KEYBOARD;
    default:
      return evrp::device::v1::DEVICE_KIND_UNSPECIFIED;
  }
}

api::DeviceKind FromProtoEnum(evrp::device::v1::DeviceKind k) {
  switch (k) {
    case evrp::device::v1::DEVICE_KIND_TOUCHPAD:
      return api::DeviceKind::kTouchpad;
    case evrp::device::v1::DEVICE_KIND_TOUCHSCREEN:
      return api::DeviceKind::kTouchscreen;
    case evrp::device::v1::DEVICE_KIND_MOUSE:
      return api::DeviceKind::kMouse;
    case evrp::device::v1::DEVICE_KIND_KEYBOARD:
      return api::DeviceKind::kKeyboard;
    default:
      return api::DeviceKind::kUnspecified;
  }
}

void ToProto(const api::InputEvent& e, evrp::device::v1::InputEvent* p) {
  p->set_device(ToProtoEnum(e.device));
  p->set_time_sec(e.time_sec);
  p->set_time_usec(e.time_usec);
  p->set_type(e.type);
  p->set_code(e.code);
  p->set_value(e.value);
}

void DrainUploadStream(
    grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* stream) {
  evrp::device::v1::UploadRecordingFrame msg;
  while (stream->Read(&msg)) {
  }
}

}  // namespace

GrpcInputDeviceService::GrpcInputDeviceService(api::IDeviceHost& host,
                                               api::IInputListener& listener)
    : host_(host), listener_(listener) {}

grpc::Status GrpcInputDeviceService::StartRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::StartRecordingRequest* request,
    google::protobuf::Empty* /*response*/) {
  if (input_session_active_.load()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "recording session already active");
  }
  std::vector<api::DeviceKind> kinds;
  kinds.reserve(static_cast<size_t>(request->kinds_size()));
  for (int i = 0; i < request->kinds_size(); ++i) {
    kinds.push_back(FromProtoEnum(request->kinds(i)));
  }
  if (!listener_.start_listening(kinds)) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "start_listening failed (no devices or already listening)");
  }
  input_read_stop_.store(false);
  input_session_active_.store(true);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadInputEvents(
    grpc::ServerContext* context,
    const google::protobuf::Empty* /*request*/,
    grpc::ServerWriter<evrp::device::v1::InputEvent>* writer) {
  if (!input_session_active_.load()) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "start_recording required before read_input_events");
  }

  while (!input_read_stop_.load() && !context->IsCancelled()) {
    std::vector<api::InputEvent> batch = listener_.read_input_events();
    for (const api::InputEvent& e : batch) {
      evrp::device::v1::InputEvent msg;
      ToProto(e, &msg);
      if (!writer->Write(msg)) {
        listener_.cancel_listening();
        input_read_stop_.store(false);
        input_session_active_.store(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "stream write failed");
      }
    }
    if (!batch.empty()) {
      continue;
    }
    if (!input_read_stop_.load() && !context->IsCancelled()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  listener_.cancel_listening();
  input_read_stop_.store(false);
  input_session_active_.store(false);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::StopRecording(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  input_read_stop_.store(true);
  listener_.cancel_listening();
  input_session_active_.store(false);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::UploadRecording(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<evrp::device::v1::UploadRecordingStatus,
                             evrp::device::v1::UploadRecordingFrame>* stream) {
  std::vector<uint8_t> middle_buf;

  auto read_next = [&](api::UploadFrame* f) -> bool {
    evrp::device::v1::UploadRecordingFrame msg;
    if (!stream->Read(&msg)) return false;
    if (msg.has_start()) {
      f->kind = api::UploadFrame::Kind::kStart;
      f->data = nullptr;
      f->data_len = 0;
      f->checksum = 0;
    } else if (msg.has_middle()) {
      f->kind = api::UploadFrame::Kind::kMiddle;
      const std::string& d = msg.middle().data();
      middle_buf.assign(d.begin(), d.end());
      f->data = middle_buf.data();
      f->data_len = middle_buf.size();
      f->checksum = msg.middle().checksum();
    } else if (msg.has_end()) {
      f->kind = api::UploadFrame::Kind::kEnd;
      f->data = nullptr;
      f->data_len = 0;
      f->checksum = 0;
    } else {
      return false;
    }
    return true;
  };

  auto emit_status = [&](const api::RecordingStatus& s) -> bool {
    evrp::device::v1::UploadRecordingStatus ps;
    ps.set_code(s.code);
    ps.set_message(s.message);
    return stream->Write(ps);
  };

  auto r = host_.upload_recording(read_next, emit_status);
  if (!r.is_ok()) {
    DrainUploadStream(stream);
  }
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::PlaybackRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* response) {
  auto r = host_.playback_recording();
  if (!r.is_ok()) return ToGrpc(r.error);
  response->set_code(r.value.code);
  response->set_message(r.value.message);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::StopPlayback(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  auto r = host_.stop_playback();
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::GetCursorPositionAvailability(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCursorPositionAvailabilityRequest* /*request*/,
    evrp::device::v1::GetCursorPositionAvailabilityResponse* response) {
  auto r = host_.get_cursor_position_availability();
  if (!r.is_ok()) return ToGrpc(r.error);
  response->set_available(r.value.available);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadCursorPosition(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::ReadCursorPositionRequest* /*request*/,
    evrp::device::v1::ReadCursorPositionResponse* response) {
  auto r = host_.read_cursor_position();
  if (!r.is_ok()) return ToGrpc(r.error);
  response->set_x(r.value.x);
  response->set_y(r.value.y);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  auto r = host_.ping();
  return ToGrpc(r.error);
}

}  // namespace evrp::device::internal
