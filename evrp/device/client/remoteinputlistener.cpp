#include "evrp/device/client/remoteinputlistener.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/deviceprotoconv.h"

namespace evrp::device::client {

RemoteInputListener::RemoteInputListener(std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)),
      stub_(evrp::device::v1::InputDeviceService::NewStub(channel_)) {}

RemoteInputListener::~RemoteInputListener() { cancel_listening(); }

bool RemoteInputListener::start_listening(
    const std::vector<api::DeviceKind>& kinds) {
  if (listening_active_) {
    return false;
  }

  evrp::device::v1::StartRecordingRequest req;
  api::ToProto(kinds, req.mutable_kinds());
  if (req.kinds_size() == 0) {
    return false;
  }

  grpc::ClientContext ctx;
  google::protobuf::Empty resp;
  grpc::Status st = stub_->StartRecording(&ctx, req, &resp);
  if (!st.ok()) {
    return false;
  }

  listening_active_ = true;
  return true;
}

std::vector<api::InputEvent> RemoteInputListener::read_input_events() {
  if (!listening_active_) {
    return {};
  }

  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  evrp::device::v1::ReadInputEventsResponse resp;
  grpc::Status st = stub_->ReadInputEvents(&ctx, req, &resp);
  if (!st.ok()) {
    return {};
  }

  std::vector<api::InputEvent> out;
  out.reserve(static_cast<size_t>(resp.events_size()));
  for (int i = 0; i < resp.events_size(); ++i) {
    api::InputEvent e;
    api::FromProto(resp.events(i), &e);
    out.push_back(std::move(e));
  }
  return out;
}

bool RemoteInputListener::wait_for_input_event(int timeout_ms) {
  if (timeout_ms < 0) {
    return false;
  }
  if (!listening_active_) {
    return false;
  }

  grpc::ClientContext ctx;
  evrp::device::v1::WaitForInputEventRequest req;
  req.set_timeout_ms(timeout_ms);
  evrp::device::v1::WaitForInputEventResponse resp;
  grpc::Status st = stub_->WaitForInputEvent(&ctx, req, &resp);
  if (!st.ok()) {
    return false;
  }
  return resp.ready();
}

void RemoteInputListener::cancel_listening() {
  bool need_stop = listening_active_;
  listening_active_ = false;
  if (!need_stop || !stub_) {
    return;
  }
  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  stub_->StopRecording(&ctx, req, &resp);
}

bool RemoteInputListener::is_listening() const { return listening_active_; }

}  // namespace evrp::device::client
