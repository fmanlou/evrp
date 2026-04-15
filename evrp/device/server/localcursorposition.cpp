#include "evrp/device/server/localcursorposition.h"

namespace evrp::device::server {

bool LocalCursorPosition::getCursorPositionAvailability() {
  return cursor_.isAvailable();
}

bool LocalCursorPosition::readCursorPosition(int *x, int *y) {
  return cursor_.getPosition(x, y);
}

}  // namespace evrp::device::server
