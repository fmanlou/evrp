#include "evrp/device/server/dispatchedinputlistener.h"

namespace evrp::device::server {

DispatchedInputListener::DispatchedInputListener(api::IInputListener& inner)
    : inner_(inner) {}

void DispatchedInputListener::shutdown() {
  syncDispatch_.shutdown([this]() { inner_.cancelListening(); });
}

DispatchedInputListener::~DispatchedInputListener() { shutdown(); }

bool DispatchedInputListener::startListening(
    const std::vector<api::DeviceKind>& kinds) {
  return syncDispatch_.postSync<bool>(
      [this, kinds]() { return inner_.startListening(kinds); });
}

std::vector<api::InputEvent> DispatchedInputListener::readInputEvents() {
  return syncDispatch_.postSync<std::vector<api::InputEvent>>(
      [this]() { return inner_.readInputEvents(); });
}

bool DispatchedInputListener::waitForInputEvent(int timeoutMs) {
  return syncDispatch_.postSync<bool>([this, timeoutMs]() {
    return inner_.waitForInputEvent(timeoutMs);
  });
}

void DispatchedInputListener::cancelListening() {
  syncDispatch_.postVoid([this]() { inner_.cancelListening(); });
}

bool DispatchedInputListener::isListening() const {
  return inner_.isListening();
}

}  // namespace evrp::device::server
