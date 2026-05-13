#pragma once

#include <vector>

#include "evrp/sdk/countingsemaphore.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/iocontextpostedbase.h"

namespace evrp::device::server {

class PostedPlayback final : public api::IPlayback,
                             private IoContextPostedBase {
 public:
  using IoContextPostedBase::shutdown;

  PostedPlayback(api::IPlayback& inner, asio::io_context& ioContext);
  ~PostedPlayback() override;

  PostedPlayback(const PostedPlayback&) = delete;
  PostedPlayback& operator=(const PostedPlayback&) = delete;

  bool upload(const std::vector<evrp::sdk::InputEvent>& events,
              evrp::sdk::StatusCode* resultOut) override;

  bool playback(evrp::sdk::StatusCode* resultOut,
                evrp::CountingSemaphore* progressNotify) override;

  int playbackIndex() const override;

  bool isPlayback() const override;

  bool stopPlayback() override;

 private:
  api::IPlayback& inner_;
};

}
