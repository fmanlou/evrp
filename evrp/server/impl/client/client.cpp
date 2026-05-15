#include "evrp/server/impl/client/client.h"

#include <utility>

#include <gflags/gflags.h>

#include "evrp/device/internal/discovery/devicediscovery.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/sessionclient.h"
#include "evrp/server/api/client.h"
#include "evrp/server/api/evrp.h"
#include "evrp/server/impl/client/remoteevrp.h"

DECLARE_string(device);
DECLARE_int32(discovery_port);
DECLARE_string(discovery_link_mode);

namespace evrp::server {

namespace {

struct ResolvedChannel {
  std::shared_ptr<grpc::Channel> channel;
  std::string server_address;
};

ResolvedChannel resolveHostGrpcChannel() {
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
        return {std::move(ch), target};
      }
    }
    logError(
        "createClient: UDP discovery found no reachable evrp gRPC targets "
        "(same defaults as --device / discovery flags).");
    return {nullptr, {}};
  }

  std::shared_ptr<grpc::Channel> ch =
      evrp::sdk::makeGrpcClientChannel(device);
  return {std::move(ch), device};
}

class ClientImpl final : public Client {
 public:
  ClientImpl(std::shared_ptr<grpc::Channel> channel, std::string server_address)
      : server_address_(std::move(server_address)),
        evrp_(std::make_unique<RemoteEvrp>(std::move(channel))) {}

  Evrp* evrp() const override { return evrp_.get(); }

  const std::string& serverAddress() const override {
    return server_address_;
  }

 private:
  std::string server_address_;
  std::unique_ptr<Evrp> evrp_;
};

}  // namespace

std::shared_ptr<grpc::Channel> createGrpcChannel() {
  return resolveHostGrpcChannel().channel;
}

std::unique_ptr<Evrp> makeEvrp(std::shared_ptr<grpc::Channel> channel) {
  if (!channel) {
    return nullptr;
  }
  return std::make_unique<RemoteEvrp>(std::move(channel));
}

std::unique_ptr<Evrp> createClient() {
  ResolvedChannel r = resolveHostGrpcChannel();
  if (!r.channel) {
    return nullptr;
  }
  return std::make_unique<RemoteEvrp>(std::move(r.channel));
}

std::unique_ptr<Client> makeClient() {
  ResolvedChannel r = resolveHostGrpcChannel();
  if (!r.channel) {
    return nullptr;
  }
  return std::make_unique<ClientImpl>(std::move(r.channel),
                                      std::move(r.server_address));
}

}  // namespace evrp::server
