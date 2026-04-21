#include "evrp/device/client/remoteinputdeviceclient.h"

#include <chrono>

#include "evrp/device/internal/tofromproto.h"

namespace evrp::device::client {

RemoteInputDeviceClient::RemoteInputDeviceClient(
    std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)),
      stub_(v1::InputDeviceService::NewStub(channel_)) {}

bool RemoteInputDeviceClient::ping() {
  grpc::ClientContext ctx;
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  v1::PingRequest req;
  v1::PingResponse resp;
  return stub_->Ping(&ctx, req, &resp).ok();
}

bool RemoteInputDeviceClient::getCapabilities(
    std::vector<api::DeviceKind>* out) {
  out->clear();
  grpc::ClientContext ctx;
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  v1::GetCapabilitiesRequest req;
  v1::GetCapabilitiesResponse resp;
  grpc::Status s = stub_->GetCapabilities(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  for (int i = 0; i < resp.supported_kinds_size(); ++i) {
    out->push_back(api::fromProto(resp.supported_kinds(i)));
  }
  return true;
}

bool RemoteInputDeviceClient::getCursorPositionAvailability(bool* available) {
  grpc::ClientContext ctx;
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  v1::GetCursorPositionAvailabilityRequest req;
  v1::GetCursorPositionAvailabilityResponse resp;
  grpc::Status s =
      stub_->GetCursorPositionAvailability(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  *available = resp.available();
  return true;
}

bool RemoteInputDeviceClient::readCursorPosition(int* outX, int* outY) {
  grpc::ClientContext ctx;
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  v1::ReadCursorPositionRequest req;
  v1::ReadCursorPositionResponse resp;
  grpc::Status s = stub_->ReadCursorPosition(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  *outX = resp.x();
  *outY = resp.y();
  return true;
}

}  // namespace evrp::device::client
