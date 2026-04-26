#pragma once

#include "evrp/sdk/cursor/cursorpos.h"
#include "evrp/device/api/cursorposition.h"

namespace evrp::device::server {

class LocalCursorPosition final : public api::ICursorPosition {
 public:
  LocalCursorPosition() = default;

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  CursorPos cursor_;
};

}
