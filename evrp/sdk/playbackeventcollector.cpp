#include "evrp/sdk/playbackeventcollector.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "evrp/device/api/playback.h"
#include "evrp/sdk/logger.h"

namespace api = evrp::device::api;

void PlaybackEventCollector::clear() {
  events_.clear();
  timelineUs_ = 0;
  hasWall_ = false;
}

void PlaybackEventCollector::appendBatchWithTimeline(
    std::vector<api::InputEvent> batch) {
  if (batch.empty()) {
    return;
  }
  const auto wall = std::chrono::steady_clock::now();
  if (hasWall_) {
    const auto gap = std::chrono::duration_cast<std::chrono::microseconds>(
        wall - lastWall_);
    const int64_t g = gap.count();
    if (g > 0) {
      timelineUs_ += g;
    }
  }
  lastWall_ = wall;
  hasWall_ = true;
  for (auto& e : batch) {
    e.timeSec = timelineUs_ / 1000000LL;
    e.timeUsec = timelineUs_ % 1000000LL;
    timelineUs_ += 1;
  }
  events_.insert(events_.end(), batch.begin(), batch.end());
}

bool PlaybackEventCollector::writeRaw(api::DeviceKind device, unsigned short type,
                                      unsigned short code, int value) {
  std::vector<api::InputEvent> batch;
  auto push = [&](unsigned short t, unsigned short c, int v) {
    api::InputEvent e;
    e.device = device;
    e.type = static_cast<uint32_t>(t);
    e.code = static_cast<uint32_t>(c);
    e.value = v;
    batch.push_back(e);
  };
  push(type, code, value);
  if (type != EV_SYN) {
    const bool needs_mt =
        (type == EV_ABS &&
         (code == ABS_MT_POSITION_Y ||
          (code == ABS_MT_TRACKING_ID && value == -1)));
    if (needs_mt) {
      push(EV_SYN, SYN_MT_REPORT, 0);
    }
    push(EV_SYN, SYN_REPORT, 0);
  }
  appendBatchWithTimeline(std::move(batch));
  return true;
}

std::vector<api::InputEvent> PlaybackEventCollector::takeEvents() {
  std::vector<api::InputEvent> out = std::move(events_);
  clear();
  return out;
}

bool PlaybackEventCollector::uploadAndPlay(api::IPlayback* playback) {
  if (!playback) {
    return false;
  }
  if (events_.empty()) {
    return true;
  }
  api::OperationResult up;
  if (!playback->upload(events_, &up) || up.code != 0) {
    logError("Playback buffer: upload failed (code={}): {}", up.code, up.message);
    return false;
  }
  api::OperationResult play;
  if (!playback->playback(&play) || play.code != 0) {
    logError("Playback buffer: playback failed (code={}): {}", play.code,
             play.message);
    return false;
  }
  clear();
  return true;
}
