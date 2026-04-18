#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <optional>
#include <thread>

#include "evrp/device/server/dispatch/dispatchedcursorposition.h"
#include "evrp/device/server/dispatch/dispatchedinputdevicekindsprovider.h"
#include "evrp/device/server/dispatch/dispatchedinputlistener.h"
#include "evrp/device/server/dispatch/dispatchedplayback.h"
#include "evrp/device/server/localcursorposition.h"
#include "evrp/device/server/localinputdevicekindsprovider.h"
#include "evrp/device/server/localinputlistener.h"
#include "evrp/device/server/localplayback.h"

namespace evrp {
class Ioc;
}

namespace evrp::device::server {

// 在独立线程上运行 asio::io_context，承载 Local* 实现；经 Dispatched* 串行化后写入 Ioc（仅 api 接口指针，不含 LocalInputListener）。
// gRPC 多线程调用均投递到该 io_context，避免与本地实现直接竞争。
class DeviceRuntime {
 public:
  DeviceRuntime();
  ~DeviceRuntime();

  DeviceRuntime(const DeviceRuntime&) = delete;
  DeviceRuntime& operator=(const DeviceRuntime&) = delete;

  void registerWith(Ioc& ioc);

 private:
  asio::io_context ioContext_;
  std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
      workGuard_;
  LocalInputListener localListener_;
  DispatchedInputListener inputListener_;
  LocalCursorPosition cursorPosition_;
  DispatchedCursorPosition dispatchedCursor_;
  LocalInputDeviceKindsProvider deviceKindsProvider_;
  DispatchedInputDeviceKindsProvider dispatchedDeviceKinds_;
  LocalPlayback playback_;
  DispatchedPlayback dispatchedPlayback_;
  std::thread worker_;
};

}  // namespace evrp::device::server
