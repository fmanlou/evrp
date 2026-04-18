#include "evrp/device/server/dispatch/dispatchedplayback.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

DispatchedPlayback::DispatchedPlayback(api::IPlayback& inner,
                                       asio::io_context& ioContext)
    : inner_(inner), syncDispatch_(ioContext) {}

DispatchedPlayback::~DispatchedPlayback() { shutdown(); }

void DispatchedPlayback::shutdown() { syncDispatch_.shutdown(); }

bool DispatchedPlayback::upload(const std::vector<api::InputEvent>& events,
                                api::OperationResult* resultOut) {
  return syncDispatch_.postSync<bool>([this, events, resultOut]() {
    return inner_.upload(events, resultOut);
  });
}

bool DispatchedPlayback::playback(api::OperationResult* resultOut,
                                  evrp::CountingSemaphore* progressNotify) {
  return syncDispatch_.postSync<bool>([this, resultOut, progressNotify]() {
    return inner_.playback(resultOut, progressNotify);
  });
}

int DispatchedPlayback::playbackIndex() const {
  return syncDispatch_.postSync<int>(
      [this]() { return inner_.playbackIndex(); });
}

bool DispatchedPlayback::stopPlayback() {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.stopPlayback(); });
}

}  // namespace evrp::device::server
