#include "evrp/device/api/deviceclient.h"

#include <google/protobuf/empty.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "evrp/device/impl/client/remoteinputdeviceclient.h"
#include "evrp/device/impl/client/remoteinputlistener.h"
#include "evrp/device/impl/client/remoteplayback.h"
#include "evrp/device/common/devicesessionmetadata.h"
#include "evrp/device/v1/service/session.grpc.pb.h"

namespace evrp::device::api {

constexpr int kKeepaliveTimeMs = 30000;
constexpr int kKeepaliveTimeoutMs = 10000;

std::shared_ptr<grpc::Channel> makeDeviceChannel(
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

bool deviceSessionConnect(const std::shared_ptr<grpc::Channel>& channel,
                          SessionInfo* out) {
  if (!out) {
    return false;
  }
  out->sessionId.clear();
  out->leaseTimeoutMs = 0;
  evrp::device::v1::DeviceSessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  google::protobuf::Empty req;
  evrp::device::v1::ConnectResponse resp;
  grpc::Status s = stub.Connect(&ctx, req, &resp);
  if (!s.ok()) {
    return false;
  }
  out->sessionId = resp.session_id();
  out->leaseTimeoutMs = resp.lease_timeout_ms();
  return !out->sessionId.empty();
}

bool deviceSessionHeartbeat(const std::shared_ptr<grpc::Channel>& channel,
                            const std::string& sessionId) {
  evrp::device::v1::DeviceSessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  evrp::device::addDeviceSessionMetadata(&ctx, sessionId);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub.Heartbeat(&ctx, req, &resp).ok();
}

bool deviceSessionDisconnect(const std::shared_ptr<grpc::Channel>& channel,
                             const std::string& sessionId) {
  evrp::device::v1::DeviceSessionService::Stub stub(channel);
  grpc::ClientContext ctx;
  evrp::device::addDeviceSessionMetadata(&ctx, sessionId);
  google::protobuf::Empty req;
  google::protobuf::Empty resp;
  return stub.Disconnect(&ctx, req, &resp).ok();
}

std::unique_ptr<IInputListener> makeRemoteInputListener(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId) {
  return std::make_unique<client::RemoteInputListener>(channel, deviceSessionId);
}

std::unique_ptr<IPlayback> makeRemotePlayback(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId) {
  return std::make_unique<client::RemotePlayback>(channel, deviceSessionId);
}

std::unique_ptr<IInputDeviceClient> makeRemoteInputDeviceClient(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId) {
  return std::make_unique<client::RemoteInputDeviceClient>(channel,
                                                           deviceSessionId);
}

}
