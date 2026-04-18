#include "evrp/device/server/posted/postedcursorposition.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedCursorPosition::PostedCursorPosition(api::ICursorPosition& inner,
                                           asio::io_context& ioContext)
    : inner_(inner), syncDispatch_(ioContext) {}

PostedCursorPosition::~PostedCursorPosition() { shutdown(); }

void PostedCursorPosition::shutdown() { syncDispatch_.shutdown(); }

bool PostedCursorPosition::getCursorPositionAvailability() {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.getCursorPositionAvailability(); });
}

bool PostedCursorPosition::readCursorPosition(int* outX, int* outY) {
  return syncDispatch_.postSync<bool>([this, outX, outY]() {
    return inner_.readCursorPosition(outX, outY);
  });
}

}  // namespace evrp::device::server
