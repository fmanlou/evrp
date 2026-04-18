#pragma once

#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/sdk/iocontextpostedbase.h"

namespace evrp::device::server {

class PostedInputListener final : public api::IInputListener,
                                  private IoContextPostedBase {
 public:
  PostedInputListener(api::IInputListener& inner, asio::io_context& ioContext);
  ~PostedInputListener() override;

  PostedInputListener(const PostedInputListener&) = delete;
  PostedInputListener& operator=(const PostedInputListener&) = delete;

  void shutdown();

  bool startListening(const std::vector<api::DeviceKind>& kinds) override;

  std::vector<api::InputEvent> readInputEvents() override;

  bool waitForInputEvent(int timeoutMs) override;

  void cancelListening() override;

  bool isListening() const override;

 private:
  api::IInputListener& inner_;
};

}  // namespace evrp::device::server
