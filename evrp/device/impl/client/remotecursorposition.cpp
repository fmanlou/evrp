#include "evrp/device/impl/client/remotecursorposition.h"

#include "evrp/device/impl/client/remoteinputdeviceclient.h"

namespace evrp::device::client {

RemoteCursorPosition::RemoteCursorPosition(RemoteInputDeviceClient* device)
    : device_(device) {}

bool RemoteCursorPosition::getCursorPositionAvailability() {
  if (!device_) {
    return false;
  }
  bool available = false;
  if (!device_->getCursorPositionAvailability(&available)) {
    return false;
  }
  return available;
}

bool RemoteCursorPosition::readCursorPosition(int* outX, int* outY) {
  if (!device_) {
    return false;
  }
  return device_->readCursorPosition(outX, outY);
}

}  // namespace evrp::device::client
