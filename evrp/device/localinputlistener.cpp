#include "evrp/device/localinputlistener.h"

namespace evrp::device {

namespace {

api::InputEvent sample_placeholder_event() {
  api::InputEvent e;
  e.device = api::DeviceKind::kKeyboard;
  e.time_sec = 0;
  e.time_usec = 0;
  e.type = 0;
  e.code = 0;
  e.value = 0;
  return e;
}

}  // namespace

bool LocalInputListener::start_listening(const std::vector<api::DeviceKind>& kinds) {
  if (listening_active_) {
    return false;
  }
  listening_active_ = true;
  kinds_ = kinds;
  return true;
}

std::vector<api::InputEvent> LocalInputListener::read_input_events() {
  if (!listening_active_) {
    return {};
  }
  std::vector<api::InputEvent> events;
  events.push_back(sample_placeholder_event());
  return events;
}

void LocalInputListener::cancel_listening() { listening_active_ = false; }

}  // namespace evrp::device
