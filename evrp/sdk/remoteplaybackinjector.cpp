#include "remoteplaybackinjector.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "evrp/device/api/playback.h"
#include "logger.h"

namespace api = evrp::device::api;

RemotePlaybackInjector::RemotePlaybackInjector(api::IPlayback* playback)
    : playback_(playback), timelineUs_(0), hasWall_(false) {}

bool RemotePlaybackInjector::flushBatch(std::vector<api::InputEvent> batch) {
  if (!playback_ || batch.empty()) {
    return true;
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
  api::OperationResult up;
  if (!playback_->upload(batch, &up) || up.code != 0) {
    logError("Remote inject: upload failed (code={}): {}", up.code, up.message);
    return false;
  }
  api::OperationResult play;
  if (!playback_->playback(&play) || play.code != 0) {
    logError("Remote inject: playback failed (code={}): {}", play.code,
             play.message);
    return false;
  }
  return true;
}

bool RemotePlaybackInjector::writeRaw(api::DeviceKind device, unsigned short type,
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
  return flushBatch(std::move(batch));
}
