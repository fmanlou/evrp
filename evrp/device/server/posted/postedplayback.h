#pragma once

#include <vector>

#include "evrp/device/api/countingsemaphore.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/syncdispatchqueue.h"

namespace evrp::device::server {

class PostedPlayback final : public api::IPlayback {
 public:
  PostedPlayback(api::IPlayback& inner, asio::io_context& ioContext);
  ~PostedPlayback() override;

  PostedPlayback(const PostedPlayback&) = delete;
  PostedPlayback& operator=(const PostedPlayback&) = delete;

  void shutdown();

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* resultOut) override;

  bool playback(api::OperationResult* resultOut,
                evrp::CountingSemaphore* progressNotify) override;

  int playbackIndex() const override;

  bool stopPlayback() override;

 private:
  api::IPlayback& inner_;
  mutable SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
