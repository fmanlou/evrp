#pragma once

#include "evrp/device/api/cursorposition.h"
#include "evrp/device/server/posted/iocontextpostedbase.h"

namespace evrp::device::server {

class PostedCursorPosition final : public api::ICursorPosition,
                                   private IoContextPostedBase {
 public:
  using IoContextPostedBase::shutdown;

  PostedCursorPosition(api::ICursorPosition& inner, asio::io_context& ioContext);
  ~PostedCursorPosition() override;

  PostedCursorPosition(const PostedCursorPosition&) = delete;
  PostedCursorPosition& operator=(const PostedCursorPosition&) = delete;

  bool getCursorPositionAvailability() override;

  bool readCursorPosition(int* outX, int* outY) override;

 private:
  api::ICursorPosition& inner_;
};

}  // namespace evrp::device::server
