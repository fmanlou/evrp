#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IInputDeviceClient {
 public:
  virtual ~IInputDeviceClient() = default;

  virtual bool getCapabilities(std::vector<DeviceKind>* out) = 0;

  virtual bool getCursorPositionAvailability(bool* available) = 0;

  virtual bool readCursorPosition(int* outX, int* outY) = 0;
};

}
