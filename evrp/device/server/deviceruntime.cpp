#include "evrp/device/server/deviceruntime.h"

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/api/inputdevicekindsprovider.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/ioc.h"

namespace evrp::device::server {

DeviceRuntime::DeviceRuntime()
    : workGuard_(asio::make_work_guard(ioContext_.get_executor())),
      localListener_(),
      inputListener_(localListener_, ioContext_),
      cursorPosition_(),
      postedCursor_(cursorPosition_, ioContext_),
      deviceKindsProvider_(),
      postedDeviceKinds_(deviceKindsProvider_, ioContext_),
      playback_(),
      postedPlayback_(playback_, ioContext_),
      worker_([this]() { ioContext_.run(); }) {}

DeviceRuntime::~DeviceRuntime() {
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

void DeviceRuntime::registerWith(Ioc& ioc) {
  ioc.emplace<api::IInputListener>(
      static_cast<api::IInputListener*>(&inputListener_));
  ioc.emplace<api::ICursorPosition>(
      static_cast<api::ICursorPosition*>(&postedCursor_));
  ioc.emplace<api::IInputDeviceKindsProvider>(
      static_cast<api::IInputDeviceKindsProvider*>(&postedDeviceKinds_));
  ioc.emplace<api::IPlayback>(
      static_cast<api::IPlayback*>(&postedPlayback_));
}

}  // namespace evrp::device::server
