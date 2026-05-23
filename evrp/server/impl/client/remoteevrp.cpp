#include "evrp/server/impl/client/remoteevrp.h"

#include <algorithm>
#include <chrono>
#include <utility>

#include <google/protobuf/empty.pb.h>
#include <google/protobuf/struct.pb.h>

#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/sessionclient.h"
#include "evrp/sdk/sessionmetadata.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

RemoteEvrp::RemoteEvrp(std::shared_ptr<grpc::Channel> channel,
                       std::string sessionId,
                       int leaseTimeoutMs)
    : channel_(std::move(channel)),
      sessionId_(std::move(sessionId)),
      leaseTimeoutMs_(leaseTimeoutMs > 0 ? leaseTimeoutMs : 3000) {
  heartbeatThread_ = std::thread([this]() { heartbeatLoop(); });
}

RemoteEvrp::~RemoteEvrp() {
  heartbeatStop_.store(true, std::memory_order_release);
  if (heartbeatThread_.joinable()) {
    heartbeatThread_.join();
  }
  if (channel_ && !sessionId_.empty()) {
    (void)evrp::sdk::sessionDisconnect(channel_, sessionId_);
  }
}

void RemoteEvrp::heartbeatLoop() {
  const int intervalMs =
      std::max(200, (leaseTimeoutMs_ * 2) / 5);
  while (!heartbeatStop_.load(std::memory_order_acquire)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    if (heartbeatStop_.load(std::memory_order_acquire)) {
      break;
    }
    if (!evrp::sdk::sessionHeartbeat(channel_, sessionId_)) {
      logWarn("evrp-client: evrp-server Session Heartbeat failed");
    }
  }
}

int RemoteEvrp::record(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  google::protobuf::Struct req;
  evrp::sdk::toProto(settings->snapshot(), &req);

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->Record(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.Record RPC failed: {}", st.error_message());
    return 1;
  }
  return resp.code();
}

int RemoteEvrp::replay(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  google::protobuf::Struct req;
  evrp::sdk::toProto(settings->snapshot(), &req);

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->Replay(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.Replay RPC failed: {}", st.error_message());
    return 1;
  }
  return resp.code();
}

bool RemoteEvrp::isRecording() const {
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  google::protobuf::Empty req;
  evrp::v1::server::BoolPayload resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->IsRecording(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.IsRecording RPC failed: {}", st.error_message());
    return false;
  }
  return resp.value();
}

bool RemoteEvrp::isReplaying() const {
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  google::protobuf::Empty req;
  evrp::v1::server::BoolPayload resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->IsReplaying(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.IsReplaying RPC failed: {}", st.error_message());
    return false;
  }
  return resp.value();
}

bool RemoteEvrp::stopRecording() {
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  google::protobuf::Empty req;
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->StopRecording(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.StopRecording RPC failed: {}",
             st.error_message());
    return false;
  }
  return resp.code() == 0;
}

bool RemoteEvrp::stopReplay() {
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId_);
  google::protobuf::Empty req;
  evrp::v1::sdk::StatusCode resp;
  auto stub = evrp::v1::server::EvrpService::NewStub(channel_);
  grpc::Status st = stub->StopReplay(&ctx, req, &resp);
  if (!st.ok()) {
    logError("EvrpService.StopReplay RPC failed: {}", st.error_message());
    return false;
  }
  return resp.code() == 0;
}

}  // namespace evrp::server
