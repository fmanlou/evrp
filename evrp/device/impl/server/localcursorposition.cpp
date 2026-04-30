#include "evrp/device/impl/server/localcursorposition.h"

namespace evrp::device::server {

LocalCursorPosition::LocalCursorPosition() : cursor_(createCursorPos()) {}

bool LocalCursorPosition::getCursorPositionAvailability() {
  return cursor_ && cursor_->isAvailable();
}

bool LocalCursorPosition::readCursorPosition(int *outX, int *outY) {
  return cursor_ && cursor_->getPosition(outX, outY);
}

}
