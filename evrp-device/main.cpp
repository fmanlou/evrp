// evrp-device：设备端进程入口。不包含 gRPC/proto 头文件。

#include <gflags/gflags.h>
#include <string>

#include "logger.h"
#include "evrp/device/server/deviceruntime.h"
#include "evrp/sdk/ioc.h"
#include "evrp/device/api/server.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address for the device service (e.g. host:port)");

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage("evrp-device");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::DeviceRuntime device;
  evrp::Ioc ioc;
  device.registerWith(ioc);
  return evrp::device::api::runDeviceServer(FLAGS_listen, ioc);
}
