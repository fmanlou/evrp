#pragma once

#include <atomic>
#include <mutex>

#include "evrp/server/api/evrp.h"

namespace evrp::device::api {
class IInputListener;
class IPlayback;
}

namespace evrp::server {

class LocalEvrp final : public Evrp {
 public:
  LocalEvrp() = default;

  int record(std::shared_ptr<ISetting> settings) override;
  int replay(std::shared_ptr<ISetting> settings) override;

  bool isRecording() const override;
  bool isReplaying() const override;
  bool stopRecording() override;
  bool stopReplay() override;

 private:
  mutable std::mutex sessionMutex_;
  evrp::device::api::IInputListener* activeListener_{nullptr};
  evrp::device::api::IPlayback* activePlayback_{nullptr};
  std::atomic<bool> isRecording_{false};
  std::atomic<bool> isReplaying_{false};
  std::atomic<bool> stopRecordingRequested_{false};
};

}  // namespace evrp::server
