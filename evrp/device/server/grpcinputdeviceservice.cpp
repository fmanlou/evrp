#include "evrp/device/server/grpcinputdeviceservice.h"

#include "evrp/device/internal/deviceprotoconv.h"

#include <google/protobuf/empty.pb.h>

#include <chrono>
#include <thread>
#include <vector>

namespace evrp::device::server {
namespace {

constexpr int k_input_wait_poll_timeout_ms = 250;

grpc::Status ToGrpc(const api::ApiError& e) {
  if (e.is_ok()) return grpc::Status::OK;
  if (e.code == 501) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, e.message);
  }
  return grpc::Status(grpc::StatusCode::UNKNOWN, e.message);
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
  if (listener_.is_listening()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "recording session already active");
  }
  std::vector<api::DeviceKind> kinds;
  api::FromProto(request->kinds(), &kinds);
  if (!listener_.start_listening(kinds)) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "start_listening failed (no devices or already listening)");
  }
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadInputEvents(
    grpc::ServerContext* context,
    const google::protobuf::Empty* /*request*/,
    grpc::ServerWriter<evrp::device::v1::InputEvent>* writer) {
  while (!context->IsCancelled()) {
    if (!listener_.is_listening()) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(k_input_wait_poll_timeout_ms));
      continue;
    }
    if (!listener_.wait_for_input_event(k_input_wait_poll_timeout_ms)) {
      if (!listener_.is_listening() || context->IsCancelled()) {
        break;
      }
      continue;
    }
    std::vector<api::InputEvent> batch = listener_.read_input_events();
    if (batch.empty()) {
      continue;
    }
    for (const api::InputEvent& e : batch) {
      evrp::device::v1::InputEvent msg;
      api::ToProto(e, &msg);
      if (!writer->Write(msg)) {
        listener_.cancel_listening();
        return grpc::Status(grpc::StatusCode::ABORTED, "stream write failed");
      }
    }
  }

  listener_.cancel_listening();
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::StopRecording(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  listener_.cancel_listening();
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

}  // namespace evrp::device::server
