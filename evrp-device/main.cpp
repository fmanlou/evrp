#include <gflags/gflags.h>

#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/impl/server/deviceruntime.h"
#include "evrp/sdk/ioc.h"
#include "logger.h"

DEFINE_string(listen, "0.0.0.0:50051",
              "Listen address for the device service (e.g. host:port; default all IPv4 interfaces)");
DEFINE_string(
    log_level, "info",
    "Log verbosity: error|warn|info|debug|trace|off (same as other evrp tools)");

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp-device");
  logService = &logSvc;

  gflags::SetUsageMessage("evrp-device");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  logService->setLevel(logLevelFromString(FLAGS_log_level));

  evrp::device::server::DeviceRuntime device;
  evrp::Ioc ioc;
  device.registerWith(ioc);
  return evrp::device::api::runDeviceServer(FLAGS_listen, ioc);
}
