#include "evrp/device/server/dispatchedinputlistener.h"

namespace evrp::device::server {

DispatchedInputListener::DispatchedInputListener(api::IInputListener& inner)
    : inner_(inner) {}

void DispatchedInputListener::shutdown() {
  sync_dispatch_.shutdown([this]() { inner_.cancel_listening(); });
}

DispatchedInputListener::~DispatchedInputListener() { shutdown(); }

bool DispatchedInputListener::start_listening(
    const std::vector<api::DeviceKind>& kinds) {
  return sync_dispatch_.post_sync<bool>(
      [this, kinds]() { return inner_.start_listening(kinds); });
}

std::vector<api::InputEvent> DispatchedInputListener::read_input_events() {
  return sync_dispatch_.post_sync<std::vector<api::InputEvent>>(
      [this]() { return inner_.read_input_events(); });
}

bool DispatchedInputListener::wait_for_input_event(int timeout_ms) {
  return sync_dispatch_.post_sync<bool>([this, timeout_ms]() {
    return inner_.wait_for_input_event(timeout_ms);
  });
}

void DispatchedInputListener::cancel_listening() {
  sync_dispatch_.post_void([this]() { inner_.cancel_listening(); });
}

bool DispatchedInputListener::is_listening() const {
  return inner_.is_listening();
}

}  // namespace evrp::device::server
