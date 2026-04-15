#include "evrp/device/client/remoteinputlistener.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::client {

RemoteInputListener::RemoteInputListener(std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)),
      stub_(evrp::device::v1::InputListenService::NewStub(channel_)) {}

RemoteInputListener::~RemoteInputListener() { cancelListening(); }

bool RemoteInputListener::startListening(
    const std::vector<api::DeviceKind>& kinds) {
  if (listeningActive_) {
    return false;
  }

  evrp::device::v1::StartRecordingRequest req;
  api::toProto(kinds, req.mutable_kinds());
  if (req.kinds_size() == 0) {
    return false;
  }

  grpc::ClientContext ctx;
  google::protobuf::Empty resp;
  grpc::Status st = stub_->StartRecording(&ctx, req, &resp);
  if (!st.ok()) {
    return false;
  }

  listeningActive_ = true;
  return true;
}

std::vector<api::InputEvent> RemoteInputListener::readInputEvents() {
  if (!listeningActive_) {
    return {};
  }

  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  evrp::device::v1::ReadInputEventsResponse resp;
  grpc::Status st = stub_->ReadInputEvents(&ctx, req, &resp);
  if (!st.ok()) {
    return {};
  }

  return api::fromProto(resp.events());
}

bool RemoteInputListener::waitForInputEvent(int timeoutMs) {
  if (timeoutMs < 0) {
    return false;
  }
  if (!listeningActive_) {
    return false;
  }

  grpc::ClientContext ctx;
  evrp::device::v1::WaitForInputEventRequest req;
  req.set_timeout_ms(timeoutMs);
  evrp::device::v1::WaitForInputEventResponse resp;
  grpc::Status st = stub_->WaitForInputEvent(&ctx, req, &resp);
  if (!st.ok()) {
    return false;
  }
  return resp.ready();
}

void RemoteInputListener::cancelListening() {
  bool needStop = listeningActive_;
  listeningActive_ = false;
  if (!needStop || !stub_) {
    return;
  }
  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  stub_->StopRecording(&ctx, req, &resp);
}

bool RemoteInputListener::isListening() const { return listeningActive_; }

}  // namespace evrp::device::client
