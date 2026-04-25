#include "evrp/device/impl/server/posted/postedplayback.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedPlayback::PostedPlayback(api::IPlayback& inner, asio::io_context& ioContext)
    : IoContextPostedBase(ioContext), inner_(inner) {}

PostedPlayback::~PostedPlayback() { shutdown(); }

bool PostedPlayback::upload(const std::vector<api::InputEvent>& events,
                              api::OperationResult* resultOut) {
  return syncDispatch_.postSync<bool>([this, events, resultOut]() {
    return inner_.upload(events, resultOut);
  });
}

bool PostedPlayback::playback(api::OperationResult* resultOut,
                              evrp::CountingSemaphore* progressNotify) {
  return syncDispatch_.postSync<bool>([this, resultOut, progressNotify]() {
    return inner_.playback(resultOut, progressNotify);
  });
}

int PostedPlayback::playbackIndex() const {
  return syncDispatch_.postSync<int>(
      [this]() { return inner_.playbackIndex(); });
}

bool PostedPlayback::stopPlayback() {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.stopPlayback(); });
}

}
