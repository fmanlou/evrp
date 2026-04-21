#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

// Host-side client for evrp.device.v1.InputDeviceService (capabilities, ping,
// cursor).
class IInputDeviceClient {
 public:
  virtual ~IInputDeviceClient() = default;

  virtual bool ping() = 0;

  virtual bool getCapabilities(std::vector<DeviceKind>* out) = 0;

  virtual bool getCursorPositionAvailability(bool* available) = 0;

  virtual bool readCursorPosition(int* outX, int* outY) = 0;
};

}  // namespace evrp::device::api
