#include "evrp/device/internal/discovery/discoveryresponder.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <thread>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/device/internal/discovery/impl/devicediscoveryprotocol.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/isetting.h"

namespace evrp::device::server {

namespace {

void discoveryResponderLoop(std::uint16_t grpcListenPort,
                            int udp_port,
                            evrp::sdk::DiscoveryLinkMode link_mode) {
  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    logError("discovery UDP: socket: {}", std::strerror(errno));
    return;
  }
  int one = 1;
  (void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  sockaddr_in bind_addr{};
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_addr.sin_port = htons(static_cast<std::uint16_t>(udp_port));
  if (bind(fd, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr)) < 0) {
    logError("discovery UDP: bind {}: {}", udp_port, std::strerror(errno));
    (void)close(fd);
    return;
  }

  if (link_mode == evrp::sdk::DiscoveryLinkMode::kMulticast) {
    ip_mreq mreq{};
    if (inet_pton(AF_INET, evrp::sdk::kDeviceDiscoveryMulticastIpv4,
                  &mreq.imr_multiaddr) != 1) {
      logError("discovery UDP: inet_pton {}",
               evrp::sdk::kDeviceDiscoveryMulticastIpv4);
      (void)close(fd);
      return;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
      logWarn(
          "discovery UDP: IP_ADD_MEMBERSHIP: {} (multicast discovery may be "
          "unavailable; loopback probe may still work)",
          std::strerror(errno));
    }
    logInfo(
        "evrp-device discovery multicast {}:{} (gRPC port {} in unicast replies)",
        evrp::sdk::kDeviceDiscoveryMulticastIpv4, udp_port, grpcListenPort);
  } else {
    logInfo(
        "evrp-device discovery broadcast on UDP {} (gRPC port {} in unicast "
        "replies)",
        udp_port, grpcListenPort);
  }

  for (;;) {
    sockaddr_in from{};
    socklen_t from_len = sizeof(from);
    evrp::sdk::DeviceDiscoveryRequest req{};
    const ssize_t n = recvfrom(fd, &req, sizeof(req), 0,
                               reinterpret_cast<sockaddr*>(&from), &from_len);
    if (n < 0) {
      continue;
    }
    if (static_cast<std::size_t>(n) != sizeof(req) ||
        !evrp::sdk::isValidDeviceDiscoveryRequest(req)) {
      continue;
    }
    evrp::sdk::DeviceDiscoveryResponse resp{};
    resp.magic[0] = 'E';
    resp.magic[1] = 'V';
    resp.magic[2] = 'R';
    resp.magic[3] = 'P';
    resp.version = evrp::sdk::kDeviceDiscoveryVersion;
    resp.reserved = 0;
    resp.grpc_port_be = htons(grpcListenPort);
    (void)sendto(fd, &resp, sizeof(resp), 0, reinterpret_cast<sockaddr*>(&from),
                 from_len);
  }
}

class UdpDiscoveryResponder final : public IDiscoveryResponder {
 public:
  explicit UdpDiscoveryResponder(const ISetting& settings) : settings_(settings) {}

  void start() override {
    const int grpc = settings_.get<int>(
        evrp::sdk::kDeviceDiscoverySettingGrpcListenPort, 0);
    if (grpc < 1 || grpc > 65535) {
      logError("discovery_grpc_listen_port invalid: {}", grpc);
      return;
    }
    const auto grpcListenPort = static_cast<std::uint16_t>(grpc);
    const int udp = settings_.get<int>(
        evrp::sdk::kDeviceDiscoverySettingPort, evrp::sdk::kDeviceDiscoveryUdpPort);
    if (udp < 1 || udp > 65535) {
      logError("discovery_port invalid: {}", udp);
      return;
    }
    evrp::sdk::DiscoveryLinkMode link_mode{};
    const std::string mode_str = settings_.get<std::string>(
        evrp::sdk::kDeviceDiscoverySettingLinkMode, std::string("broadcast"));
    if (!evrp::sdk::tryParseDiscoveryLinkMode(mode_str, &link_mode)) {
      logError(
          "discovery_link_mode invalid: {} (expected multicast or broadcast)",
          mode_str);
      return;
    }
    std::thread([grpcListenPort, udp, link_mode]() {
      discoveryResponderLoop(grpcListenPort, udp, link_mode);
    }).detach();
  }

 private:
  const ISetting& settings_;
};

}

std::unique_ptr<IDiscoveryResponder> createDiscoveryResponder(
    const ISetting& settings) {
  return std::make_unique<UdpDiscoveryResponder>(settings);
}

}
