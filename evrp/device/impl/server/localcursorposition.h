#pragma once

#include "evrp/sdk/cursor/cursorpos.h"
#include "evrp/device/api/cursorposition.h"

namespace evrp::device::server {

class LocalCursorPosition final : public api::ICursorPosition {
 public:
  LocalCursorPosition();
  ~LocalCursorPosition() override = default;

  LocalCursorPosition(const LocalCursorPosition &) = delete;
  LocalCursorPosition &operator=(const LocalCursorPosition &) = delete;

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int *outX, int *outY) override;

 private:
  ICursorPos *cursor_{nullptr};
};

}
