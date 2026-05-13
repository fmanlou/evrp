#include "evrp/server/impl/client/client.h"

#include <utility>

#include <gflags/gflags.h>

#include "evrp/device/internal/discovery/devicediscovery.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/sessionclient.h"
#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/client/remoteevrp.h"

DECLARE_string(device);
DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

namespace evrp::server {

namespace {

std::shared_ptr<grpc::Channel> resolveHostGrpcChannel() {
  MemorySetting discoveryOpts;
  discoveryOpts.insert("device", std::string(FLAGS_device));
  discoveryOpts.insert(evrp::sdk::kDeviceDiscoverySettingPort,
                       FLAGS_discovery_port);
  discoveryOpts.insert(evrp::sdk::kDeviceDiscoverySettingLinkMode,
                       FLAGS_discovery_link_mode);

  std::string device = FLAGS_device;
  if (evrp::device::api::useUdpDeviceDiscovery(device)) {
    const std::unique_ptr<evrp::device::api::IUdpDeviceDiscoverer> discoverer =
        evrp::device::api::createUdpDeviceDiscoverer(discoveryOpts);
    for (const std::string& target : discoverer->discoverGrpcTargets()) {
      std::shared_ptr<grpc::Channel> ch =
          evrp::sdk::makeGrpcClientChannel(target);
      if (ch) {
        return ch;
      }
    }
    logError(
        "createClient: UDP discovery found no reachable evrp gRPC targets "
        "(same defaults as --device / discovery flags).");
    return nullptr;
  }

  return evrp::sdk::makeGrpcClientChannel(device);
}

}  // namespace

std::shared_ptr<grpc::Channel> createGrpcChannel() {
  return resolveHostGrpcChannel();
}

std::unique_ptr<Evrp> makeEvrp(std::shared_ptr<grpc::Channel> channel) {
  if (!channel) {
    return nullptr;
  }
  return std::make_unique<RemoteEvrp>(std::move(channel));
}

std::unique_ptr<Evrp> createClient() {
  return makeEvrp(createGrpcChannel());
}

}  // namespace evrp::server
