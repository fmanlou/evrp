#include "evrp/device/impl/server/localcursorposition.h"

namespace evrp::device::server {

bool LocalCursorPosition::getCursorPositionAvailability() {
  return cursor_.isAvailable();
}

bool LocalCursorPosition::readCursorPosition(int* outX, int* outY) {
  return cursor_.getPosition(outX, outY);
}

}
