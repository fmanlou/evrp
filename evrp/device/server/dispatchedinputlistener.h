#pragma once

#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/server/syncdispatchqueue.h"

namespace evrp::device::server {

class DispatchedInputListener final : public api::IInputListener {
 public:
  explicit DispatchedInputListener(api::IInputListener& inner);
  ~DispatchedInputListener() override;

  DispatchedInputListener(const DispatchedInputListener&) = delete;
  DispatchedInputListener& operator=(const DispatchedInputListener&) = delete;

  void shutdown();

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  api::IInputListener& inner_;
  SyncDispatchQueue syncDispatch_;
};

}  // namespace evrp::device::server
