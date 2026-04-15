#pragma once

#include "cursor/cursorpos.h"
#include "evrp/device/api/cursorposition.h"

namespace evrp::device::server {

class LocalCursorPosition final : public api::ICursorPosition {
 public:
  LocalCursorPosition() = default;

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int *x, int *y) override;

 private:
  CursorPos cursor_;
};

}  // namespace evrp::device::server
