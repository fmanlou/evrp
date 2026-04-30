#pragma once

#include <atomic>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/sdk/filesystem.h"

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

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  struct TrackedDevice {
    int fd{-1};
    api::DeviceKind kind{api::DeviceKind::kUnspecified};
    std::string path;
  };

  void closeDevices();
  std::string listenDevicesSummary() const;

  EnhancedFileSystem fs_;
  std::mutex mu_;
  std::atomic<bool> listeningActive_{false};
  std::atomic<bool> disposed_{false};
  std::vector<TrackedDevice> devices_;
  std::set<size_t> pollReadyIndices_;
};

}
