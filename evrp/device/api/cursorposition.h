#pragma once

namespace evrp::device::api {

class ICursorPosition {
 public:
  virtual ~ICursorPosition() = default;

  virtual bool getCursorPositionAvailability() = 0;

  virtual bool readCursorPosition(int *x, int *y) = 0;
};

}  // namespace evrp::device::api
