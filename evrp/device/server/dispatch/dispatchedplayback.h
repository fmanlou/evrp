#pragma once

#include <vector>

#include "evrp/device/api/countingsemaphore.h"
#include "evrp/device/api/playback.h"
#include "evrp/sdk/syncdispatchqueue.h"

namespace evrp::device::server {

class DispatchedPlayback final : public api::IPlayback {
 public:
  DispatchedPlayback(api::IPlayback& inner, asio::io_context& ioContext);
  ~DispatchedPlayback() override;

  DispatchedPlayback(const DispatchedPlayback&) = delete;
  DispatchedPlayback& operator=(const DispatchedPlayback&) = delete;

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
