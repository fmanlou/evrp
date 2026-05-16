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

std::unique_ptr<RemoteEvrp> connectRemoteEvrp(
    std::shared_ptr<grpc::Channel> channel) {
  evrp::sdk::SessionInfo info;
  if (!evrp::sdk::sessionConnect(channel, &info) ||
      info.sessionId.empty()) {
    logError(
        "evrp-server: SessionService.Connect failed (is evrp-server running "
        "and compatible?)");
    return nullptr;
  }
  return std::make_unique<RemoteEvrp>(
      std::move(channel), std::move(info.sessionId), info.leaseTimeoutMs);
}

class ClientImpl final : public Client {
 public:
  ClientImpl(std::string server_address, std::unique_ptr<RemoteEvrp> evrp)
      : server_address_(std::move(server_address)),
        evrp_(std::move(evrp)) {}

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
  std::unique_ptr<RemoteEvrp> remote = connectRemoteEvrp(std::move(channel));
  if (!remote) {
    return nullptr;
  }
  return std::unique_ptr<Evrp>(remote.release());
}

std::unique_ptr<Evrp> createClient() {
  ResolvedChannel r = resolveHostGrpcChannel();
  if (!r.channel) {
    return nullptr;
  }
  return makeEvrp(std::move(r.channel));
}

std::unique_ptr<Client> makeClient() {
  ResolvedChannel r = resolveHostGrpcChannel();
  if (!r.channel) {
    return nullptr;
  }
  std::unique_ptr<RemoteEvrp> evrp = connectRemoteEvrp(std::move(r.channel));
  if (!evrp) {
    return nullptr;
  }
  return std::make_unique<ClientImpl>(std::move(r.server_address),
                                        std::move(evrp));
}

std::unique_ptr<Client> makeClient(std::string evrp_service_address) {
  if (evrp_service_address.empty()) {
    return nullptr;
  }
  std::shared_ptr<grpc::Channel> ch =
      evrp::sdk::makeGrpcClientChannel(evrp_service_address);
  if (!ch) {
    return nullptr;
  }
  std::unique_ptr<RemoteEvrp> evrp = connectRemoteEvrp(std::move(ch));
  if (!evrp) {
    return nullptr;
  }
  return std::make_unique<ClientImpl>(std::move(evrp_service_address),
                                        std::move(evrp));
}

}  // namespace evrp::server
