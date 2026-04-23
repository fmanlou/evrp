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

struct SessionInfo {
  std::string sessionId;
  int leaseTimeoutMs = 0;
};

std::shared_ptr<grpc::Channel> makeDeviceChannel(const std::string& targetHostPort);

bool deviceSessionConnect(const std::shared_ptr<grpc::Channel>& channel,
                          SessionInfo* out);
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

}
