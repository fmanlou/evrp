// IInputListener 端到端测试：DispatchedInputListener(LocalInputListener) + 完整 gRPC（与 evrp-device 等价）。

#include <gflags/gflags.h>
#include <string>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/server/dispatchedinputlistener.h"
#include "evrp/device/server/localcursorposition.h"
#include "evrp/device/server/localinputlistener.h"
#include "evrp/device/server/localinputdevicekindsprovider.h"
#include "evrp/device/server/localplayback.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for InputListenService and peers");

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage(
      "evrp_inputlisten_test_server — DispatchedInputListener + gRPC");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::LocalInputListener local_listener;
  evrp::device::server::DispatchedInputListener input_listener(local_listener);
  evrp::device::server::LocalCursorPosition cursor_position;
  evrp::device::server::LocalInputDeviceKindsProvider device_kinds_provider;
  evrp::device::server::LocalPlayback playback;
  return evrp::device::api::runDeviceServer(
      FLAGS_listen, &input_listener, &cursor_position, &device_kinds_provider,
      &playback);
}
