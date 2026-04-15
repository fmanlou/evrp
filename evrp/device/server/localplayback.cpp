#include "evrp/device/server/localplayback.h"

#include <chrono>
#include <thread>

#include "filesystem.h"
#include "inputeventwriter.h"

namespace evrp::device::server {

bool LocalPlayback::upload(const std::vector<api::InputEvent>& events,
                           api::OperationResult* result_out) {
  std::lock_guard<std::mutex> lock(mu_);
  cached_ = events;
  playing_ = false;
  if (result_out) {
    result_out->code = 0;
    result_out->message.clear();
  }
  return true;
}

int LocalPlayback::playbackIndex() const { return current_event_index_; }

bool LocalPlayback::playback(
    api::OperationResult* result_out,
    evrp::CountingSemaphore* progress_notify) {
  std::vector<api::InputEvent> batch;
  {
    std::lock_guard<std::mutex> lock(mu_);
    if (cached_.empty()) {
      if (result_out) {
        result_out->code = 1;
        result_out->message = "no recording uploaded";
      }
      return false;
    }
    batch = cached_;
    playing_ = true;
  }
  stop_requested_ = false;
  current_event_index_ = -1;

  FileSystem fs;
  InputEventWriter writer(&fs);

  bool first = true;
  int64_t prev_us = 0;
  for (int i = 0; i < static_cast<int>(batch.size()); ++i) {
    const api::InputEvent& e = batch[static_cast<size_t>(i)];
    if (stop_requested_) {
      break;
    }

    int64_t t_us = static_cast<int64_t>(e.time_sec) * 1000000LL +
                   static_cast<int64_t>(e.time_usec);
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
      if (result_out) {
        result_out->code = 2;
        result_out->message = "write event failed during playback";
      }
      return false;
    }
    current_event_index_ = i;
    if (progress_notify != nullptr) {
      progress_notify->release();
    }
  }

  {
    std::lock_guard<std::mutex> lock(mu_);
    playing_ = false;
  }
  if (result_out) {
    result_out->code = 0;
    result_out->message.clear();
  }
  return true;
}

bool LocalPlayback::stopPlayback() {
  stop_requested_ = true;
  std::lock_guard<std::mutex> lock(mu_);
  playing_ = false;
  return true;
}

}  // namespace evrp::device::server
