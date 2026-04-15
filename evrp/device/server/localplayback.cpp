#include "evrp/device/server/localplayback.h"

#include <chrono>
#include <thread>

#include "filesystem.h"
#include "inputeventwriter.h"

namespace evrp::device::server {

bool LocalPlayback::upload(const std::vector<api::InputEvent>& events,
                           api::OperationResult* resultOut) {
  std::lock_guard<std::mutex> lock(mu_);
  cached_ = events;
  playing_ = false;
  if (resultOut) {
    resultOut->code = 0;
    resultOut->message.clear();
  }
  return true;
}

int LocalPlayback::playbackIndex() const { return currentEventIndex_; }

bool LocalPlayback::playback(
    api::OperationResult* resultOut,
    evrp::CountingSemaphore* progressNotify) {
  std::vector<api::InputEvent> batch;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (cached_.empty()) {
      if (resultOut) {
        resultOut->code = 1;
        resultOut->message = "no recording uploaded";
      }
      return false;
    }
    batch = cached_;
    playing_ = true;
  }
  stopRequested_ = false;
  currentEventIndex_ = -1;

  FileSystem fs;
  InputEventWriter writer(&fs);

  bool first = true;
  int64_t prev_us = 0;
  for (int i = 0; i < static_cast<int>(batch.size()); ++i) {
    const api::InputEvent& e = batch[static_cast<size_t>(i)];
    if (stopRequested_) {
      break;
    }

    int64_t t_us = static_cast<int64_t>(e.timeSec) * 1000000LL +
                   static_cast<int64_t>(e.timeUsec);
    if (!first) {
      int64_t delta = t_us - prev_us;
      if (delta > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(delta));
      }
    }
    first = false;
    prev_us = t_us;

    if (!writer.write(e.device, static_cast<unsigned short>(e.type),
                      static_cast<unsigned short>(e.code), static_cast<int>(e.value))) {
      {
        std::lock_guard<std::mutex> lock(mu_);
        playing_ = false;
      }
      if (resultOut) {
        resultOut->code = 2;
        resultOut->message = "write event failed during playback";
      }
      return false;
    }
    currentEventIndex_ = i;
    if (progressNotify != nullptr) {
      progressNotify->release();
    }
  }

  {
    std::lock_guard<std::mutex> lock(mu_);
    playing_ = false;
  }
  if (resultOut) {
    resultOut->code = 0;
    resultOut->message.clear();
  }
  return true;
}

bool LocalPlayback::stopPlayback() {
  stopRequested_ = true;
  std::lock_guard<std::mutex> lock(mu_);
  playing_ = false;
  return true;
}

}  // namespace evrp::device::server
