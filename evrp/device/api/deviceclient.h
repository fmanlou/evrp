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

// Insecure channel to evrp-device; disables HTTP proxy so localhost / device
// IPs are not sent to http_proxy.
std::shared_ptr<grpc::Channel> makeDeviceChannel(const std::string& targetHostPort);

std::unique_ptr<IInputListener> makeRemoteInputListener(
    const std::shared_ptr<grpc::Channel>& channel);

std::unique_ptr<IPlayback> makeRemotePlayback(
    const std::shared_ptr<grpc::Channel>& channel);

std::unique_ptr<IInputDeviceClient> makeRemoteInputDeviceClient(
    const std::shared_ptr<grpc::Channel>& channel);

}  // namespace evrp::device::api
