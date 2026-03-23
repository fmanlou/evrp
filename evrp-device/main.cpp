// evrp-device：设备端进程入口。仅依赖 api 层（server.h / IDeviceHost），不包含传输实现头文件。

#include <gflags/gflags.h>

#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/stub_device_host.h"

DEFINE_string(listen, "127.0.0.1:50051", "Listen address for the device service (e.g. host:port)");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("evrp-device");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::StubDeviceHost host;
  return evrp::device::api::RunDeviceServer(FLAGS_listen, host);
}
