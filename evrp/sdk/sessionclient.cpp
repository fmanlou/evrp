#include "evrp/sdk/sessionclient.h"

#include <google/protobuf/empty.pb.h>

#include <chrono>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "evrp/sdk/sessionmetadata.h"
#include "evrp/sdk/v1/services/session.grpc.pb.h"

namespace evrp::sdk {

namespace {

constexpr int kKeepaliveTimeMs = 30000;
constexpr int kKeepaliveTimeoutMs = 10000;

}

std::shared_ptr<grpc::Channel> makeGrpcClientChannel(
    const std::string& targetHostPort) {
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_ENABLE_HTTP_PROXY, 0);
  args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, kKeepaliveTimeMs);
  args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, kKeepaliveTimeoutMs);
  args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
  args.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
  return grpc::CreateCustomChannel(targetHostPort,
                                   grpc::InsecureChannelCredentials(), args);
}

bool sessionConnect(const std::shared_ptr<grpc::Channel>& channel,
                    SessionInfo* out) {
  if (!out) {
    return false;
  }
  out->sessionId.clear();
  out->leaseTimeoutMs = 0;
  evrp::sdk::v1::SessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  evrp::sdk::v1::ConnectResponse resp;
  grpc::Status s = stub.Connect(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  out->sessionId = resp.session_id();
  out->leaseTimeoutMs = resp.lease_timeout_ms();
  return !out->sessionId.empty();
}

bool sessionConnectWithDeadline(const std::shared_ptr<grpc::Channel>& channel,
                                SessionInfo* out,
                                std::chrono::milliseconds rpc_timeout) {
  if (!out) {
    return false;
  }
  out->sessionId.clear();
  out->leaseTimeoutMs = 0;
  evrp::sdk::v1::SessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  ctx.set_deadline(std::chrono::system_clock::now() + rpc_timeout);
  google::protobuf::Empty req;
  evrp::sdk::v1::ConnectResponse resp;
  grpc::Status s = stub.Connect(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  out->sessionId = resp.session_id();
  out->leaseTimeoutMs = resp.lease_timeout_ms();
  return !out->sessionId.empty();
}

bool sessionHeartbeat(const std::shared_ptr<grpc::Channel>& channel,
                      const std::string& sessionId) {
  evrp::sdk::v1::SessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub.Heartbeat(&ctx, req, &resp).ok();
}

bool sessionDisconnect(const std::shared_ptr<grpc::Channel>& channel,
                       const std::string& sessionId) {
  evrp::sdk::v1::SessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, sessionId);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub.Disconnect(&ctx, req, &resp).ok();
}

}
