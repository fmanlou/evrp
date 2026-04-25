#include "evrp/device/impl/server/posted/postedcursorposition.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedCursorPosition::PostedCursorPosition(api::ICursorPosition& inner,
                                           asio::io_context& ioContext)
    : IoContextPostedBase(ioContext), inner_(inner) {}

PostedCursorPosition::~PostedCursorPosition() { shutdown(); }

bool PostedCursorPosition::getCursorPositionAvailability() {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.getCursorPositionAvailability(); });
}

bool PostedCursorPosition::readCursorPosition(int* outX, int* outY) {
  return syncDispatch_.postSync<bool>([this, outX, outY]() {
    return inner_.readCursorPosition(outX, outY);
  });
}

}
