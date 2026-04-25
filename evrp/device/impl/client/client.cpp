#include "evrp/device/api/client.h"

#include "evrp/device/impl/client/remoteinputdeviceclient.h"
#include "evrp/device/impl/client/remoteinputlistener.h"
#include "evrp/device/impl/client/remoteplayback.h"

namespace evrp::device::api {

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
