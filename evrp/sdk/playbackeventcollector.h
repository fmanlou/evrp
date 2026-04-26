#pragma once

#include <chrono>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/iraweventwriter.h"

namespace evrp::device::api {
class IPlayback;
}

/// Buffers evdev-shaped events from an IRawEventWriter sink, then sends them in
/// one upload + playback to the device (after Lua or other logic finishes).
class PlaybackEventCollector final : public IRawEventWriter {
 public:
  void clear();

  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value) override;

  bool uploadAndPlay(evrp::device::api::IPlayback* playback);

  /// Moves buffered events out and resets the collector (timeline + buffer).
  std::vector<evrp::device::api::InputEvent> takeEvents();

 private:
  void appendBatchWithTimeline(std::vector<evrp::device::api::InputEvent> batch);

  std::vector<evrp::device::api::InputEvent> events_;
  int64_t timelineUs_ = 0;
  bool hasWall_ = false;
  std::chrono::steady_clock::time_point lastWall_{};
};
