#include "grpc/grpc_input_device_service.h"

#include <google/protobuf/empty.pb.h>

#include <vector>

namespace evrp {
namespace device {
namespace {

grpc::Status ToGrpc(const api::ApiError& e) {
  if (e.ok()) return grpc::Status::OK;
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

GrpcInputDeviceService::GrpcInputDeviceService(api::IDeviceHost& host) : host_(host) {}

grpc::Status GrpcInputDeviceService::StartReadInput(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::StartReadInputRequest* request,
    google::protobuf::Empty* /*response*/) {
  std::vector<api::DeviceKind> kinds;
  kinds.reserve(static_cast<size_t>(request->kinds_size()));
  for (int i = 0; i < request->kinds_size(); ++i) {
    kinds.push_back(FromProtoEnum(request->kinds(i)));
  }
  auto r = host_.StartReadInput(kinds);
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::ReadInputEvents(
    grpc::ServerContext* /*context*/,
    const google::protobuf::Empty* /*request*/,
    grpc::ServerWriter<evrp::device::v1::InputEvent>* writer) {
  auto r = host_.ReadInputEvents([&](const api::InputEvent& e) {
    evrp::device::v1::InputEvent msg;
    ToProto(e, &msg);
    writer->Write(msg);
  });
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::StopReadInput(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  auto r = host_.StopReadInput();
  return ToGrpc(r.error);
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

  auto r = host_.UploadRecording(read_next, emit_status);
  if (!r.ok()) {
    DrainUploadStream(stream);
  }
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::PlaybackRecording(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::PlaybackRecordingRequest* /*request*/,
    evrp::device::v1::PlaybackRecordingResponse* response) {
  auto r = host_.PlaybackRecording();
  if (!r.ok()) return ToGrpc(r.error);
  response->set_code(r.value.code);
  response->set_message(r.value.message);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::StopPlayback(
    grpc::ServerContext* /*context*/, const google::protobuf::Empty* /*request*/,
    google::protobuf::Empty* /*response*/) {
  auto r = host_.StopPlayback();
  return ToGrpc(r.error);
}

grpc::Status GrpcInputDeviceService::GetCursorPositionAvailability(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::GetCursorPositionAvailabilityRequest* /*request*/,
    evrp::device::v1::GetCursorPositionAvailabilityResponse* response) {
  auto r = host_.GetCursorPositionAvailability();
  if (!r.ok()) return ToGrpc(r.error);
  response->set_available(r.value.available);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::ReadCursorPosition(
    grpc::ServerContext* /*context*/,
    const evrp::device::v1::ReadCursorPositionRequest* /*request*/,
    evrp::device::v1::ReadCursorPositionResponse* response) {
  auto r = host_.ReadCursorPosition();
  if (!r.ok()) return ToGrpc(r.error);
  response->set_x(r.value.x);
  response->set_y(r.value.y);
  return grpc::Status::OK;
}

grpc::Status GrpcInputDeviceService::Ping(grpc::ServerContext* /*context*/,
                                          const evrp::device::v1::PingRequest* /*request*/,
                                          evrp::device::v1::PingResponse* /*response*/) {
  auto r = host_.Ping();
  return ToGrpc(r.error);
}

}  // namespace device
}  // namespace evrp
