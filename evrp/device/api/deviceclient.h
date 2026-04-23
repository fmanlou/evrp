#pragma once

#include <memory>
#include <string>

#include "evrp/device/api/inputdeviceclient.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"

namespace grpc {
class Channel;
}

namespace evrp::device::api {

struct DeviceSessionInfo {
  std::string sessionId;
  int leaseTimeoutMs = 0;
};

// Insecure channel to evrp-device; disables HTTP proxy so localhost / device
// IPs are not sent to http_proxy. Enables HTTP/2 keepalive (see
// deviceclient.cpp; server uses matching args in grpcserverimpl.cpp).
std::shared_ptr<grpc::Channel> makeDeviceChannel(const std::string& targetHostPort);

// Business session: call Connect before other device RPCs; keep Heartbeat within
// lease_timeout_ms. Pass sessionId into makeRemote* helpers.
bool deviceSessionConnect(const std::shared_ptr<grpc::Channel>& channel,
                          DeviceSessionInfo* out);
bool deviceSessionHeartbeat(const std::shared_ptr<grpc::Channel>& channel,
                            const std::string& sessionId);
bool deviceSessionDisconnect(const std::shared_ptr<grpc::Channel>& channel,
                             const std::string& sessionId);

std::unique_ptr<IInputListener> makeRemoteInputListener(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId);

std::unique_ptr<IPlayback> makeRemotePlayback(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId);

std::unique_ptr<IInputDeviceClient> makeRemoteInputDeviceClient(
    const std::shared_ptr<grpc::Channel>& channel,
    const std::string& deviceSessionId);

}  // namespace evrp::device::api
