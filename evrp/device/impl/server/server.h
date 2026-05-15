#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <memory>
#include <optional>
#include <thread>

#include "evrp/device/api/server.h"
#include "evrp/device/impl/server/grpcserver.h"
#include "evrp/device/impl/server/localcursorposition.h"
#include "evrp/device/impl/server/localinputdevicekindsprovider.h"
#include "evrp/device/impl/server/localinputlistener.h"
#include "evrp/device/impl/server/localplayback.h"
#include "evrp/device/impl/server/posted/postedcursorposition.h"
#include "evrp/device/impl/server/posted/postedinputdevicekindsprovider.h"
#include "evrp/device/impl/server/posted/postedinputlistener.h"
#include "evrp/device/impl/server/posted/postedplayback.h"
#include "evrp/sdk/ioc.h"

class ISetting;

namespace evrp::device::api {

class Server final : public IServer {
 public:
  explicit Server(const ISetting& deviceSettings);
  ~Server() override;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  int run() override;

 private:
  evrp::Ioc ioc_;

  asio::io_context ioContext_;
  std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
      workGuard_;
  std::unique_ptr<evrp::device::server::LocalInputListener> localListener_;
  std::unique_ptr<evrp::device::server::PostedInputListener> inputListener_;
  std::unique_ptr<evrp::device::server::LocalCursorPosition> cursorPosition_;
  std::unique_ptr<evrp::device::server::PostedCursorPosition> postedCursor_;
  std::unique_ptr<evrp::device::server::LocalInputDeviceKindsProvider>
      deviceKindsProvider_;
  std::unique_ptr<evrp::device::server::PostedInputDeviceKindsProvider>
      postedDeviceKinds_;
  std::unique_ptr<evrp::device::server::LocalPlayback> playback_;
  std::unique_ptr<evrp::device::server::PostedPlayback> postedPlayback_;
  std::thread worker_;

  std::unique_ptr<evrp::device::server::GrpcServer> grpcServer_;
};

}  // namespace evrp::device::api
