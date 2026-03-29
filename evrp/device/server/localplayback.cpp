#include "evrp/device/server/localplayback.h"

#include <chrono>
#include <thread>

#include "deviceid.h"
#include "filesystem.h"
#include "inputeventwriter.h"

namespace evrp::device::server {

namespace {

DeviceId device_id_from_api_kind(api::DeviceKind k) {
  switch (k) {
    case api::DeviceKind::kTouchpad:
      return DeviceId::Touchpad;
    case api::DeviceKind::kTouchscreen:
      return DeviceId::Touchscreen;
    case api::DeviceKind::kMouse:
      return DeviceId::Mouse;
    case api::DeviceKind::kKeyboard:
      return DeviceId::Keyboard;
    default:
      return DeviceId::Unknown;
  }
}

}  // namespace

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

bool LocalPlayback::playback(api::OperationResult* result_out) {
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
  stop_requested_.store(false, std::memory_order_relaxed);

  FileSystem fs;
  InputEventWriter writer(&fs);

  bool first = true;
  int64_t prev_us = 0;
  for (const api::InputEvent& e : batch) {
    if (stop_requested_.load(std::memory_order_relaxed)) {
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

    DeviceId id = device_id_from_api_kind(e.device);
    if (!writer.write(id, static_cast<unsigned short>(e.type),
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

bool LocalPlayback::stop_playback() {
  stop_requested_.store(true, std::memory_order_relaxed);
  std::lock_guard<std::mutex> lock(mu_);
  playing_ = false;
  return true;
}

}  // namespace evrp::device::server
