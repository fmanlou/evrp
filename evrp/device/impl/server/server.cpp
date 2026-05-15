#include "evrp/device/impl/server/server.h"

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/devicekindsprovider.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/setting/isetting.h"

namespace evrp::device::api {

namespace detail {

Server::Server(const ISetting& deviceSettings)
    : ioc_(),
      ioContext_(),
      workGuard_(asio::make_work_guard(ioContext_.get_executor())),
      localListener_(),
      inputListener_(localListener_, ioContext_),
      cursorPosition_(),
      postedCursor_(cursorPosition_, ioContext_),
      deviceKindsProvider_(),
      postedDeviceKinds_(deviceKindsProvider_, ioContext_),
      playback_(),
      postedPlayback_(playback_, ioContext_),
      worker_([this]() { ioContext_.run(); }),
      grpcServer_(std::make_unique<evrp::device::server::GrpcServer>(
          ioc_, deviceSettings)) {
  ioc_.emplace<IInputListener>(
      static_cast<IInputListener*>(&inputListener_));
  ioc_.emplace<ICursorPosition>(
      static_cast<ICursorPosition*>(&postedCursor_));
  ioc_.emplace<IDeviceKindsProvider>(
      static_cast<IDeviceKindsProvider*>(&postedDeviceKinds_));
  ioc_.emplace<IPlayback>(static_cast<IPlayback*>(&postedPlayback_));
}

Server::~Server() {
  grpcServer_.reset();
  inputListener_.shutdown();
  postedCursor_.shutdown();
  postedDeviceKinds_.shutdown();
  postedPlayback_.shutdown();
  workGuard_.reset();
  ioContext_.stop();
  if (worker_.joinable()) {
    worker_.join();
  }
}

int Server::run() { return grpcServer_->run(); }

}  // namespace detail

std::unique_ptr<IServer> makeServer(const ISetting& device_settings) {
  return std::make_unique<detail::Server>(device_settings);
}

}  // namespace evrp::device::api
