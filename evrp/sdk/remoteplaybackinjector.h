#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "evrp/device/api/types.h"
#include "iraweventwriter.h"

namespace evrp::device::api {
class IPlayback;
}

/// Sends evdev-shaped events through IPlayback (upload + playback per logical
/// flush). Used by clients; device-side local injection uses InputEventWriter.
class RemotePlaybackInjector final : public IRawEventWriter {
 public:
  explicit RemotePlaybackInjector(evrp::device::api::IPlayback* playback);
  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value) override;

 private:
  bool flushBatch(std::vector<evrp::device::api::InputEvent> batch);

  evrp::device::api::IPlayback* playback_;
  int64_t timelineUs_;
  bool hasWall_;
  std::chrono::steady_clock::time_point lastWall_;
};
