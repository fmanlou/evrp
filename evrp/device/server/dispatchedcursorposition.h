#pragma once

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/server/syncdispatchqueue.h"

namespace evrp::device::server {

class DispatchedCursorPosition final : public api::ICursorPosition {
 public:
  DispatchedCursorPosition(api::ICursorPosition& inner, asio::io_context& ioContext);
  ~DispatchedCursorPosition() override;

  DispatchedCursorPosition(const DispatchedCursorPosition&) = delete;
  DispatchedCursorPosition& operator=(const DispatchedCursorPosition&) = delete;

  void shutdown();

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  api::ICursorPosition& inner_;
  mutable SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
