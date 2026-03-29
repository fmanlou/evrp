// IInputListener 端到端测试：DispatchedInputListener(LocalInputListener) + 完整 gRPC（与 evrp-device 等价）。

#include <gflags/gflags.h>
#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/server/dispatchedinputlistener.h"
#include "evrp/device/server/localinputlistener.h"
#include "evrp/device/server/localplayback.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for InputListenService and peers");

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
      "evrp_inputlisten_test_server — DispatchedInputListener + gRPC");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::LocalInputListener local_listener;
  evrp::device::server::DispatchedInputListener input_listener(local_listener);
  evrp::device::server::LocalPlayback playback;
  return evrp::device::api::run_device_server(FLAGS_listen, input_listener,
                                              playback);
}
