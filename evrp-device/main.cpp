#include <gflags/gflags.h>

#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/impl/server/deviceruntime.h"
#include "evrp/device/internal/discovery/devicediscoveryprotocol.h"
#include "evrp/sdk/ioc.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

DEFINE_string(listen, "0.0.0.0:50051",
              "Listen address for the device service (e.g. host:port; default all IPv4 interfaces)");
DEFINE_string(
    log_level, "info",
    "Log verbosity: error|warn|info|debug|trace|off (same as other evrp tools)");

DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp-device");
  logService = &logSvc;

  gflags::SetUsageMessage("evrp-device");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  logService->setLevel(logLevelFromString(FLAGS_log_level));

  MemorySetting device_settings;
  device_settings.insert(evrp::sdk::kDeviceDiscoverySettingPort,
                         FLAGS_discovery_port);
  device_settings.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode,
                         FLAGS_discovery_link_mode);

  evrp::device::server::DeviceRuntime device;
  evrp::Ioc ioc;
  device.registerWith(ioc);
  return evrp::device::api::makeServer(FLAGS_listen, ioc, device_settings)->run();
}
