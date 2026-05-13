#include "evrp/device/impl/client/remoteinputlistener.h"

#include <google/protobuf/empty.pb.h>

#include "evrp/sdk/sessionmetadata.h"
#include "evrp/sdk/fmt/fmt.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/sdk/logger.h"

namespace evrp::device::client {

RemoteInputListener::RemoteInputListener(std::shared_ptr<grpc::Channel> channel,
                                         std::string deviceSessionId)
    : stub_(evrp::v1::device::InputListenService::NewStub(std::move(channel))),
      deviceSessionId_(std::move(deviceSessionId)) {}

RemoteInputListener::~RemoteInputListener() { cancelListening(); }

bool RemoteInputListener::startListening(
    const std::vector<evrp::sdk::DeviceKind>& kinds) {
  if (listeningActive_) {
    return false;
  }

  evrp::v1::device::StartRecordingRequest req;
  evrp::sdk::toProto(kinds, req.mutable_kinds());
  if (req.kinds_size() == 0) {
    return false;
  }

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  google::protobuf::Empty resp;
  grpc::Status st = stub_->StartRecording(&ctx, req, &resp);
  if (!st.ok()) {
    logError(
        "RemoteInputListener: StartRecording gRPC code={} {}",
        st.error_code(),
        st.error_message());
    return false;
  }

  listeningActive_ = true;
  return true;
}

std::vector<evrp::sdk::InputEvent> RemoteInputListener::readInputEvents() {
  if (!listeningActive_) {
    return {};
  }

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  google::protobuf::Empty req;
  evrp::v1::device::ReadInputEventsResponse resp;
  grpc::Status st = stub_->ReadInputEvents(&ctx, req, &resp);
  if (!st.ok()) {
    return {};
  }

  return evrp::sdk::fromProto(resp.events());
}

bool RemoteInputListener::waitForInputEvent(int timeoutMs) {
  if (timeoutMs < 0) {
    return false;
  }
  if (!listeningActive_) {
    return false;
  }

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  evrp::v1::device::WaitForInputEventRequest req;
  req.set_timeout_ms(timeoutMs);
  evrp::v1::device::WaitForInputEventResponse resp;
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
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  stub_->StopRecording(&ctx, req, &resp);
}

bool RemoteInputListener::isListening() const { return listeningActive_; }

}
