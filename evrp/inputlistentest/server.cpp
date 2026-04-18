#include <gflags/gflags.h>
#include <string>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/server/deviceruntime.h"
#include "evrp/ioc.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for InputListenService and peers");

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage(
      "evrp_inputlisten_test_server — DispatchedInputListener + gRPC");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::DeviceRuntime device;
  evrp::Ioc ioc;
  device.registerWith(ioc);
  return evrp::device::api::runDeviceServer(FLAGS_listen, ioc);
}
