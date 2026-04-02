#pragma once

#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/server/syncdispatchqueue.h"

namespace evrp::device::server {

// 装饰器：经 `SyncDispatchQueue` 在单线程上调用被包装的 `IInputListener`。
class DispatchedInputListener final : public api::IInputListener {
 public:
  explicit DispatchedInputListener(api::IInputListener& inner);
  ~DispatchedInputListener() override;

  DispatchedInputListener(const DispatchedInputListener&) = delete;
  DispatchedInputListener& operator=(const DispatchedInputListener&) = delete;

  void shutdown();

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeout_ms) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  api::IInputListener& inner_;
  SyncDispatchQueue sync_dispatch_;
};

}  // namespace evrp::device::server
