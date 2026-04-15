#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IInputListener {
 public:
  virtual ~IInputListener() = default;

  virtual bool startListening(const std::vector<DeviceKind>& kinds) = 0;

  virtual std::vector<InputEvent> readInputEvents() = 0;

  virtual bool waitForInputEvent(int timeoutMs) = 0;

  virtual void cancelListening() = 0;

  virtual bool isListening() const = 0;
};

}  // namespace evrp::device::api
