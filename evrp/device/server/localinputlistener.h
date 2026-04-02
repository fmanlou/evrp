#pragma once

#include <atomic>
#include <mutex>
#include <set>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "filesystem.h"

namespace evrp::device::server {

class LocalInputListener final : public api::IInputListener {
 public:
  LocalInputListener() = default;

  void dispose();

  ~LocalInputListener() override;

  LocalInputListener(const LocalInputListener&) = delete;
  LocalInputListener& operator=(const LocalInputListener&) = delete;

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeout_ms) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  struct TrackedDevice {
    int fd{-1};
    api::DeviceKind kind{api::DeviceKind::kUnspecified};
  };

  void closeDevices();

  FileSystem fs_;
  std::mutex mu_;
  std::atomic<bool> listening_active_{false};
  std::atomic<bool> disposed_{false};
  std::vector<TrackedDevice> devices_;
  std::set<size_t> poll_ready_indices_;
};

}
