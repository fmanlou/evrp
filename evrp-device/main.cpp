// evrp-device：设备端进程入口。依赖 server/server.h、stubdevicehost，不包含 gRPC/proto 头文件。

#include <gflags/gflags.h>

#include <string>

#include "evrp/device/server/server.h"
#include "evrp/device/server/localinputlistener.h"
#include "evrp/device/stubdevicehost.h"

DEFINE_string(listen, "127.0.0.1:50051", "Listen address for the device service (e.g. host:port)");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("evrp-device");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::StubDeviceHost host;
  evrp::device::server::LocalInputListener input_listener;
  return evrp::device::api::run_device_server(FLAGS_listen, host, input_listener);
}
