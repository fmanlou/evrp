#include "evrp/server/impl/client/remoteevrp.h"

#include <utility>

#include <google/protobuf/empty.pb.h>
#include <google/protobuf/struct.pb.h>

#include "evrp/sdk/logger.h"
#include "evrp/sdk/tofromproto.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

namespace evrp::server {

RemoteEvrp::RemoteEvrp(std::shared_ptr<grpc::Channel> channel)
    : channel_(std::move(channel)) {}

int RemoteEvrp::record(std::shared_ptr<ISetting> settings) {
  if (!settings) {
    logError("settings is null.");
    return 1;
  }
  google::protobuf::Struct req;
  evrp::sdk::toProto(settings->snapshot(), &req);

  grpc::ClientContext ctx;
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
