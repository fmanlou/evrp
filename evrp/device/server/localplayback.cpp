#include "evrp/device/server/localplayback.h"

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

bool LocalPlayback::playback(api::OperationResult* result_out) {
  std::lock_guard<std::mutex> lock(mu_);
  if (cached_.empty()) {
    if (result_out) {
      result_out->code = 1;
      result_out->message = "no recording uploaded";
    }
    return false;
  }
  playing_ = true;
  if (result_out) {
    result_out->code = 0;
    result_out->message.clear();
  }
  return true;
}

bool LocalPlayback::stop_playback() {
  std::lock_guard<std::mutex> lock(mu_);
  playing_ = false;
  return true;
}

}  // namespace evrp::device::server
