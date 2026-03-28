// IInputListener 端到端测试：本进程内 LocalInputListener + 完整 gRPC 设备服务（与 evrp-device 等价）。

#include <gflags/gflags.h>
#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/server/localinputlistener.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for InputListenService and peers");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("evrp_inputlisten_test_server — LocalInputListener + gRPC");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::LocalInputListener input_listener;
  return evrp::device::api::run_device_server(FLAGS_listen, input_listener);
}
