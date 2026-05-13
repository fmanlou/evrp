#pragma once

#include "evrp/device/api/cursorposition.h"

namespace evrp::device::client {

class RemoteInputDeviceClient;

class RemoteCursorPosition final : public api::ICursorPosition {
 public:
  explicit RemoteCursorPosition(RemoteInputDeviceClient* device);

  RemoteCursorPosition(const RemoteCursorPosition&) = delete;
  RemoteCursorPosition& operator=(const RemoteCursorPosition&) = delete;

  bool getCursorPositionAvailability() override;
  bool readCursorPosition(int* outX, int* outY) override;

 private:
  RemoteInputDeviceClient* device_{};
};

}  // namespace evrp::device::client
