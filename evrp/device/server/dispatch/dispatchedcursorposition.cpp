#include "evrp/device/server/dispatch/dispatchedcursorposition.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

DispatchedCursorPosition::DispatchedCursorPosition(api::ICursorPosition& inner,
                                                   asio::io_context& ioContext)
    : inner_(inner), syncDispatch_(ioContext) {}

DispatchedCursorPosition::~DispatchedCursorPosition() { shutdown(); }

void DispatchedCursorPosition::shutdown() { syncDispatch_.shutdown(); }

bool DispatchedCursorPosition::getCursorPositionAvailability() {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.getCursorPositionAvailability(); });
}

bool DispatchedCursorPosition::readCursorPosition(int* outX, int* outY) {
  return syncDispatch_.postSync<bool>([this, outX, outY]() {
    return inner_.readCursorPosition(outX, outY);
  });
}

}  // namespace evrp::device::server
