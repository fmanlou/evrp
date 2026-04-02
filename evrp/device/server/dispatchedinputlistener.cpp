#include "evrp/device/server/dispatchedinputlistener.h"

namespace evrp::device::server {

DispatchedInputListener::DispatchedInputListener(api::IInputListener& inner)
    : inner_(inner) {}

void DispatchedInputListener::shutdown() {
  sync_dispatch_.shutdown([this]() { inner_.cancelListening(); });
}

DispatchedInputListener::~DispatchedInputListener() { shutdown(); }

bool DispatchedInputListener::startListening(
    const std::vector<api::DeviceKind>& kinds) {
  return sync_dispatch_.postSync<bool>(
      [this, kinds]() { return inner_.startListening(kinds); });
}

std::vector<api::InputEvent> DispatchedInputListener::readInputEvents() {
  return sync_dispatch_.postSync<std::vector<api::InputEvent>>(
      [this]() { return inner_.readInputEvents(); });
}

bool DispatchedInputListener::waitForInputEvent(int timeout_ms) {
  return sync_dispatch_.postSync<bool>([this, timeout_ms]() {
    return inner_.waitForInputEvent(timeout_ms);
  });
}

void DispatchedInputListener::cancelListening() {
  sync_dispatch_.postVoid([this]() { inner_.cancelListening(); });
}

bool DispatchedInputListener::isListening() const {
  return inner_.isListening();
}

}  // namespace evrp::device::server
