#include "evrp/device/api/deviceclient.h"

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "evrp/device/client/remoteinputdeviceclient.h"
#include "evrp/device/client/remoteinputlistener.h"
#include "evrp/device/client/remoteplayback.h"

namespace evrp::device::api {

std::shared_ptr<grpc::Channel> makeDeviceChannel(
    const std::string& targetHostPort) {
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_ENABLE_HTTP_PROXY, 0);
  return grpc::CreateCustomChannel(targetHostPort,
                                   grpc::InsecureChannelCredentials(), args);
}

std::unique_ptr<IInputListener> makeRemoteInputListener(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::make_unique<client::RemoteInputListener>(channel);
}

std::unique_ptr<IPlayback> makeRemotePlayback(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::make_unique<client::RemotePlayback>(channel);
}

std::unique_ptr<IInputDeviceClient> makeRemoteInputDeviceClient(
    const std::shared_ptr<grpc::Channel>& channel) {
  return std::make_unique<client::RemoteInputDeviceClient>(channel);
}

}  // namespace evrp::device::api
