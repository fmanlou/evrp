#pragma once

#include <vector>

#include "evrp/device/api/types.h"

namespace evrp::device::api {

class IInputListener {
 public:
  virtual ~IInputListener() = default;

  virtual bool start_listening(const std::vector<DeviceKind>& kinds) = 0;

  virtual std::vector<InputEvent> read_input_events() = 0;

  virtual bool wait_for_input_event() = 0;

  virtual void cancel_listening() = 0;

  virtual bool is_listening() const = 0;
};

}  // namespace evrp::device::api
