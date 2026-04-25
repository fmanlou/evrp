#include "evrp/device/api/client.h"

#include <grpcpp/grpcpp.h>

#include "evrp/sdk/sessionclient.h"
#include "evrp/device/impl/client/remoteinputdeviceclient.h"
#include "evrp/device/impl/client/remoteinputlistener.h"
#include "evrp/device/impl/client/remoteplayback.h"

namespace evrp::device::api {

namespace {

class ClientImpl final : public IClient {
 public:
  static std::unique_ptr<ClientImpl> create(const std::string& targetHostPort) {
    if (targetHostPort.empty()) {
      return nullptr;
    }
    std::shared_ptr<grpc::Channel> ch =
        evrp::sdk::makeGrpcClientChannel(targetHostPort);
    if (!ch) {
      return nullptr;
    }
    evrp::sdk::SessionInfo info;
    if (!evrp::sdk::sessionConnect(ch, &info) || info.sessionId.empty()) {
      return nullptr;
    }
    return std::unique_ptr<ClientImpl>(
        new ClientImpl(std::move(ch), std::move(info.sessionId)));
  }

  IInputListener* inputListener() const final { return listener_.get(); }
  IPlayback* playback() const final { return playback_.get(); }
  IInputDeviceClient* inputDevice() const final { return inputDevice_.get(); }

  ~ClientImpl() override {
    if (channel_ && !sessionId_.empty()) {
      (void)evrp::sdk::sessionDisconnect(channel_, sessionId_);
    }
  }

  ClientImpl(const ClientImpl&) = delete;
  ClientImpl& operator=(const ClientImpl&) = delete;
  ClientImpl(ClientImpl&&) noexcept = default;
  ClientImpl& operator=(ClientImpl&&) noexcept = default;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::string sessionId_;
  std::unique_ptr<IInputListener> listener_;
  std::unique_ptr<IPlayback> playback_;
  std::unique_ptr<IInputDeviceClient> inputDevice_;

  explicit ClientImpl(std::shared_ptr<grpc::Channel> channel, std::string sessionId)
      : channel_(std::move(channel)),
        sessionId_(std::move(sessionId)),
        listener_(
            std::make_unique<client::RemoteInputListener>(channel_, sessionId_)),
        playback_(
            std::make_unique<client::RemotePlayback>(channel_, sessionId_)),
        inputDevice_(
            std::make_unique<client::RemoteInputDeviceClient>(channel_,
                                                              sessionId_)) {}
};

}  // namespace

std::unique_ptr<IClient> makeClient(const std::string& targetHostPort) {
  return ClientImpl::create(targetHostPort);
}

}  // namespace evrp::device::api
