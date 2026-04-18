#include "evrp/device/server/posted/postedinputlistener.h"

#include <asio/io_context.hpp>

namespace evrp::device::server {

PostedInputListener::PostedInputListener(api::IInputListener& inner,
                                         asio::io_context& ioContext)
    : IoContextPostedBase(ioContext), inner_(inner) {}

void PostedInputListener::shutdown() {
  syncDispatch_.postVoid([this]() { inner_.cancelListening(); });
  IoContextPostedBase::shutdown();
}

PostedInputListener::~PostedInputListener() { shutdown(); }

bool PostedInputListener::startListening(
    const std::vector<api::DeviceKind>& kinds) {
  return syncDispatch_.postSync<bool>(
      [this, kinds]() { return inner_.startListening(kinds); });
}

std::vector<api::InputEvent> PostedInputListener::readInputEvents() {
  return syncDispatch_.postSync<std::vector<api::InputEvent>>(
      [this]() { return inner_.readInputEvents(); });
}

bool PostedInputListener::waitForInputEvent(int timeoutMs) {
  return syncDispatch_.postSync<bool>([this, timeoutMs]() {
    return inner_.waitForInputEvent(timeoutMs);
  });
}

void PostedInputListener::cancelListening() {
  syncDispatch_.postVoid([this]() { inner_.cancelListening(); });
}

bool PostedInputListener::isListening() const {
  return syncDispatch_.postSync<bool>(
      [this]() { return inner_.isListening(); });
}

}  // namespace evrp::device::server
