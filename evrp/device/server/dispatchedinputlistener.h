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

  bool start_listening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> read_input_events() override;

  bool wait_for_input_event(int timeout_ms) override;

  void cancel_listening() override;

  bool is_listening() const override;

 private:
  api::IInputListener& inner_;
  SyncDispatchQueue sync_dispatch_;
};

}  // namespace evrp::device::server
