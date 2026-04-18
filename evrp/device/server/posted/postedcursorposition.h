#pragma once

#include "evrp/device/api/cursorposition.h"
#include "evrp/sdk/syncdispatchqueue.h"

namespace evrp::device::server {

class PostedCursorPosition final : public api::ICursorPosition {
 public:
  PostedCursorPosition(api::ICursorPosition& inner, asio::io_context& ioContext);
  ~PostedCursorPosition() override;

  PostedCursorPosition(const PostedCursorPosition&) = delete;
  PostedCursorPosition& operator=(const PostedCursorPosition&) = delete;

  void shutdown();

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  api::ICursorPosition& inner_;
  mutable SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
