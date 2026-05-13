#include "evrp/device/impl/client/remoteinputdeviceclient.h"

#include <chrono>

#include "evrp/sdk/sessionmetadata.h"
#include "evrp/sdk/tofromproto.h"

namespace evrp::device::client {

RemoteInputDeviceClient::RemoteInputDeviceClient(
    std::shared_ptr<grpc::Channel> channel,
    std::string deviceSessionId)
    : channel_(std::move(channel)),
      stub_(evrp::v1::device::InputDeviceService::NewStub(channel_)),
      deviceSessionId_(std::move(deviceSessionId)) {}

bool RemoteInputDeviceClient::getCapabilities(
    std::vector<evrp::sdk::DeviceKind>* out) {
  out->clear();
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  evrp::v1::device::GetCapabilitiesRequest req;
  evrp::v1::device::GetCapabilitiesResponse resp;
  grpc::Status s = stub_->GetCapabilities(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  for (int i = 0; i < resp.supported_kinds_size(); ++i) {
    out->push_back(evrp::sdk::fromProto(resp.supported_kinds(i)));
  }
  return true;
}

bool RemoteInputDeviceClient::getCursorPositionAvailability(bool* available) {
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  evrp::v1::device::GetCursorPositionAvailabilityRequest req;
  evrp::v1::device::GetCursorPositionAvailabilityResponse resp;
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
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);
  ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));
  evrp::v1::device::ReadCursorPositionRequest req;
  evrp::v1::device::ReadCursorPositionResponse resp;
  grpc::Status s = stub_->ReadCursorPosition(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  *outX = resp.x();
  *outY = resp.y();
  return true;
}

}
