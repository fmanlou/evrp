#include "evrp/server/impl/client/remoteevrp.h"

#include <utility>

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

}  // namespace evrp::server
