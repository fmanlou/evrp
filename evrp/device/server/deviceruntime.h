#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <optional>
#include <thread>

#include "evrp/device/server/localcursorposition.h"
#include "evrp/device/server/localinputdevicekindsprovider.h"
#include "evrp/device/server/localinputlistener.h"
#include "evrp/device/server/localplayback.h"
#include "evrp/device/server/posted/postedcursorposition.h"
#include "evrp/device/server/posted/postedinputdevicekindsprovider.h"
#include "evrp/device/server/posted/postedinputlistener.h"
#include "evrp/device/server/posted/postedplayback.h"

namespace evrp {
class Ioc;
}

namespace evrp::device::server {

// 在独立线程上运行 asio::io_context，承载 Local* 实现；经 Posted* 投递到 io_context 串行执行后写入 Ioc（仅 api 接口指针，不含 LocalInputListener）。
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
  PostedInputListener inputListener_;
  LocalCursorPosition cursorPosition_;
  PostedCursorPosition postedCursor_;
  LocalInputDeviceKindsProvider deviceKindsProvider_;
  PostedInputDeviceKindsProvider postedDeviceKinds_;
  LocalPlayback playback_;
  PostedPlayback postedPlayback_;
  std::thread worker_;
};

}  // namespace evrp::device::server
