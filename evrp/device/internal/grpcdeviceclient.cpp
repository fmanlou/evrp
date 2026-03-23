// gRPC 客户端实现：仅在本翻译单元包含 gRPC / proto 头文件。

#include "evrp/device/api/client_factory.h"

#include <google/protobuf/empty.pb.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>

#include "evrp/device/v1/device.grpc.pb.h"

namespace evrp::device::api {
namespace {

evrp::device::v1::DeviceKind ToProtoEnum(DeviceKind k) {
  switch (k) {
    case DeviceKind::kTouchpad:
      return evrp::device::v1::DEVICE_KIND_TOUCHPAD;
    case DeviceKind::kTouchscreen:
      return evrp::device::v1::DEVICE_KIND_TOUCHSCREEN;
    case DeviceKind::kMouse:
      return evrp::device::v1::DEVICE_KIND_MOUSE;
    case DeviceKind::kKeyboard:
      return evrp::device::v1::DEVICE_KIND_KEYBOARD;
    default:
      return evrp::device::v1::DEVICE_KIND_UNSPECIFIED;
  }
}

void FromProto(const evrp::device::v1::InputEvent& p, InputEvent* e) {
  switch (p.device()) {
    case evrp::device::v1::DEVICE_KIND_TOUCHPAD:
      e->device = DeviceKind::kTouchpad;
      break;
    case evrp::device::v1::DEVICE_KIND_TOUCHSCREEN:
      e->device = DeviceKind::kTouchscreen;
      break;
    case evrp::device::v1::DEVICE_KIND_MOUSE:
      e->device = DeviceKind::kMouse;
      break;
    case evrp::device::v1::DEVICE_KIND_KEYBOARD:
      e->device = DeviceKind::kKeyboard;
      break;
    default:
      e->device = DeviceKind::kUnspecified;
      break;
  }
  e->time_sec = p.time_sec();
  e->time_usec = p.time_usec();
  e->type = p.type();
  e->code = p.code();
  e->value = p.value();
}

ApiError FromGrpcStatus(const grpc::Status& s) {
  if (s.ok()) return ApiError::success();
  return ApiError::make(static_cast<int>(s.error_code()), s.error_message());
}

template <typename T>
ApiResult<T> fail_result(const grpc::Status& s) {
  ApiResult<T> r;
  r.error = FromGrpcStatus(s);
  return r;
}

ApiResult<void> fail_result_void(const grpc::Status& s) {
  ApiResult<void> r;
  r.error = FromGrpcStatus(s);
  return r;
}

void FillUploadProto(const UploadFrame& f, evrp::device::v1::UploadRecordingFrame* msg) {
  msg->Clear();
  switch (f.kind) {
    case UploadFrame::Kind::kStart:
      msg->mutable_start();
      break;
    case UploadFrame::Kind::kMiddle:
      msg->mutable_middle()->set_data(
          reinterpret_cast<const char*>(f.data),
          static_cast<size_t>(f.data_len));
      msg->mutable_middle()->set_checksum(f.checksum);
      break;
    case UploadFrame::Kind::kEnd:
      msg->mutable_end();
      break;
  }
}

class GrpcDeviceClient final : public IDeviceClient {
 public:
  explicit GrpcDeviceClient(std::shared_ptr<grpc::Channel> channel)
      : channel_(std::move(channel)),
        stub_(evrp::device::v1::InputDeviceService::NewStub(channel_)) {}

  ApiResult<void> ping() override {
    grpc::ClientContext ctx;
    evrp::device::v1::PingRequest req;
    evrp::device::v1::PingResponse resp;
    grpc::Status s = stub_->Ping(&ctx, req, &resp);
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<void> start_recording(const std::vector<DeviceKind>& kinds) override {
    grpc::ClientContext ctx;
    evrp::device::v1::StartRecordingRequest req;
    for (DeviceKind k : kinds) {
      req.add_kinds(ToProtoEnum(k));
    }
    google::protobuf::Empty resp;
    grpc::Status s = stub_->StartRecording(&ctx, req, &resp);
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<void> read_input_events(
      const std::function<void(const InputEvent&)>& on_event) override {
    grpc::ClientContext ctx;
    google::protobuf::Empty req;
    std::unique_ptr<grpc::ClientReader<evrp::device::v1::InputEvent>> reader(
        stub_->ReadInputEvents(&ctx, req));
    evrp::device::v1::InputEvent pe;
    while (reader->Read(&pe)) {
      InputEvent e;
      FromProto(pe, &e);
      on_event(e);
    }
    grpc::Status s = reader->Finish();
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<void> stop_recording() override {
    grpc::ClientContext ctx;
    google::protobuf::Empty req;
    google::protobuf::Empty resp;
    grpc::Status s = stub_->StopRecording(&ctx, req, &resp);
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<void> upload_recording(
      const std::function<bool(UploadFrame* frame)>& next_frame_to_send,
      const std::function<void(const RecordingStatus&)>& on_status) override {
    grpc::ClientContext ctx;
    auto stream = stub_->UploadRecording(&ctx);
    UploadFrame f{};
    while (next_frame_to_send(&f)) {
      evrp::device::v1::UploadRecordingFrame msg;
      FillUploadProto(f, &msg);
      if (!stream->Write(msg)) break;
    }
    stream->WritesDone();
    evrp::device::v1::UploadRecordingStatus st;
    while (stream->Read(&st)) {
      RecordingStatus rs;
      rs.code = st.code();
      rs.message = st.message();
      on_status(rs);
    }
    grpc::Status s = stream->Finish();
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<PlaybackResponse> playback_recording() override {
    grpc::ClientContext ctx;
    evrp::device::v1::PlaybackRecordingRequest req;
    evrp::device::v1::PlaybackRecordingResponse resp;
    grpc::Status s = stub_->PlaybackRecording(&ctx, req, &resp);
    if (!s.ok()) return fail_result<PlaybackResponse>(s);
    PlaybackResponse out;
    out.code = resp.code();
    out.message = resp.message();
    ApiResult<PlaybackResponse> r;
    r.value = out;
    return r;
  }

  ApiResult<void> stop_playback() override {
    grpc::ClientContext ctx;
    google::protobuf::Empty req;
    google::protobuf::Empty resp;
    grpc::Status s = stub_->StopPlayback(&ctx, req, &resp);
    if (!s.ok()) return fail_result_void(s);
    return ApiResult<void>{};
  }

  ApiResult<CursorAvailability> get_cursor_position_availability() override {
    grpc::ClientContext ctx;
    evrp::device::v1::GetCursorPositionAvailabilityRequest req;
    evrp::device::v1::GetCursorPositionAvailabilityResponse resp;
    grpc::Status s = stub_->GetCursorPositionAvailability(&ctx, req, &resp);
    if (!s.ok()) return fail_result<CursorAvailability>(s);
    ApiResult<CursorAvailability> r;
    r.value.available = resp.available();
    return r;
  }

  ApiResult<CursorPosition> read_cursor_position() override {
    grpc::ClientContext ctx;
    evrp::device::v1::ReadCursorPositionRequest req;
    evrp::device::v1::ReadCursorPositionResponse resp;
    grpc::Status s = stub_->ReadCursorPosition(&ctx, req, &resp);
    if (!s.ok()) return fail_result<CursorPosition>(s);
    ApiResult<CursorPosition> r;
    r.value.x = resp.x();
    r.value.y = resp.y();
    return r;
  }

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<evrp::device::v1::InputDeviceService::Stub> stub_;
};

}  // namespace

std::unique_ptr<IDeviceClient> connect_device_client(const std::string& target) {
  auto channel =
      grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
  return std::make_unique<GrpcDeviceClient>(std::move(channel));
}

}  // namespace evrp::device::api
