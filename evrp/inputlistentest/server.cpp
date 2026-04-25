#include <gflags/gflags.h>
#include <string>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/impl/server/deviceruntime.h"
#include "evrp/sdk/ioc.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for InputListenService and peers");

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp_inputlisten_test_server");
  logService = &logSvc;

  gflags::SetUsageMessage(
      "evrp_inputlisten_test_server — PostedInputListener + gRPC");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  evrp::device::server::DeviceRuntime device;
  evrp::Ioc ioc;
  device.registerWith(ioc);
  return evrp::device::api::makeServer()->run(FLAGS_listen, ioc);
}
