#include "evrp/device/internal/discovery/devicediscovery.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cerrno>
#include <cstring>

#include <gflags/gflags.h>
#include <algorithm>
#include <set>
#include <unordered_set>

#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/device/internal/discovery/impl/devicediscoveryprotocol.h"
#include "evrp/sdk/setting/isetting.h"

DEFINE_int32(
    discovery_port, evrp::sdk::kDeviceDiscoveryUdpPort,
    "UDP port for IPv4 device discovery (empty --device; "
    "evrp-device --discovery_port must match).");
DEFINE_string(
    discovery_link_mode, "multicast",
    "Discovery probes: \"multicast\" (default) or \"broadcast\"; must match "
    "evrp-device.");

namespace evrp::device::api {

namespace {

bool sendDiscoveryProbe(int fd, const sockaddr* addr, socklen_t len) {
  evrp::sdk::DeviceDiscoveryRequest req{};
  evrp::sdk::fillDeviceDiscoveryRequest(&req);
  return sendto(fd, &req, sizeof(req), 0, addr, len) == static_cast<ssize_t>(sizeof(req));
}

std::unordered_set<std::string> collectLocalIpv4Addresses() {
  std::unordered_set<std::string> out;
  ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) == -1) {
    return out;
  }
  for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    if (!(ifa->ifa_flags & IFF_UP)) {
      continue;
    }
    const auto* sin = reinterpret_cast<const sockaddr_in*>(ifa->ifa_addr);
    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)) == nullptr) {
      continue;
    }
    if (std::strcmp(buf, "0.0.0.0") == 0) {
      continue;
    }
    out.insert(buf);
  }
  freeifaddrs(ifaddr);
  return out;
}

std::string hostOfIpv4HostPort(const std::string& host_port) {
  const auto colon = host_port.rfind(':');
  if (colon == std::string::npos) {
    return {};
  }
  return host_port.substr(0, colon);
}

int localTargetRank(const std::string& target,
                    const std::unordered_set<std::string>& local_ipv4) {
  const std::string host = hostOfIpv4HostPort(target);
  if (host == "127.0.0.1") {
    return 0;
  }
  if (local_ipv4.count(host) != 0) {
    return 1;
  }
  return 2;
}

void sortDiscoveredTargetsPreferSameHost(
    std::vector<std::string>* targets,
    const std::unordered_set<std::string>& local_ipv4) {
  std::stable_sort(
      targets->begin(), targets->end(),
      [&local_ipv4](const std::string& a, const std::string& b) {
        return localTargetRank(a, local_ipv4) < localTargetRank(b, local_ipv4);
      });
}

bool prepareDiscoverySocketForMode(int fd, evrp::sdk::DiscoveryLinkMode mode) {
  if (mode == evrp::sdk::DiscoveryLinkMode::kMulticast) {
    const unsigned char mcast_ttl = 1;
    return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &mcast_ttl,
                      sizeof(mcast_ttl)) == 0;
  }
  int broadcast = 1;
  return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast,
                    sizeof(broadcast)) == 0;
}

void transmitDiscoveryProbes(int fd,
                             std::uint16_t udp_be,
                             evrp::sdk::DiscoveryLinkMode mode) {
  sockaddr_in loop{};
  loop.sin_family = AF_INET;
  loop.sin_port = udp_be;
  loop.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  (void)sendDiscoveryProbe(fd, reinterpret_cast<sockaddr*>(&loop),
                           sizeof(loop));

  if (mode == evrp::sdk::DiscoveryLinkMode::kMulticast) {
    sockaddr_in mcast{};
    mcast.sin_family = AF_INET;
    mcast.sin_port = udp_be;
    if (inet_pton(AF_INET, evrp::sdk::kDeviceDiscoveryMulticastIpv4,
                  &mcast.sin_addr) != 1) {
      return;
    }
    (void)sendDiscoveryProbe(fd, reinterpret_cast<sockaddr*>(&mcast),
                             sizeof(mcast));
    return;
  }

  sockaddr_in global_bcast{};
  global_bcast.sin_family = AF_INET;
  global_bcast.sin_port = udp_be;
  global_bcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  (void)sendDiscoveryProbe(fd, reinterpret_cast<sockaddr*>(&global_bcast),
                             sizeof(global_bcast));

  ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) != 0) {
    return;
  }
  for (ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_flags & IFF_LOOPBACK) {
      continue;
    }
    if (!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_BROADCAST)) {
      continue;
    }
    if (ifa->ifa_broadaddr == nullptr ||
        ifa->ifa_broadaddr->sa_family != AF_INET) {
      continue;
    }
    auto* b = reinterpret_cast<sockaddr_in*>(ifa->ifa_broadaddr);
    b->sin_port = udp_be;
    (void)sendDiscoveryProbe(fd, reinterpret_cast<sockaddr*>(b), sizeof(*b));
  }
  freeifaddrs(ifaddr);
}

std::vector<std::string> discoverGrpcTargetsImpl(
    int discovery_udp_port, evrp::sdk::DiscoveryLinkMode link_mode) {
  std::vector<std::string> targets;
  if (discovery_udp_port < 1 || discovery_udp_port > 65535) {
    return targets;
  }

  const std::unordered_set<std::string> local_ipv4 = collectLocalIpv4Addresses();

  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    return targets;
  }

  (void)prepareDiscoverySocketForMode(fd, link_mode);

  const std::uint16_t udp_be = htons(static_cast<std::uint16_t>(discovery_udp_port));

  std::set<std::string> seen;
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(1500);
  auto next_resend = std::chrono::steady_clock::time_point{};

  while (std::chrono::steady_clock::now() < deadline) {
    const auto now = std::chrono::steady_clock::now();
    if (now >= next_resend) {
      transmitDiscoveryProbes(fd, udp_be, link_mode);
      next_resend = now + std::chrono::milliseconds(200);
    }

    const auto remain_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now)
            .count();
    if (remain_ms <= 0) {
      break;
    }
    timeval tv{};
    const int slice_ms = std::min<int64_t>(remain_ms, 250);
    tv.tv_sec = static_cast<long>(slice_ms / 1000);
    tv.tv_usec = static_cast<long>((slice_ms % 1000) * 1000);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    const int sel = select(fd + 1, &rfds, nullptr, nullptr, &tv);
    if (sel < 0) {
      break;
    }
    if (sel == 0) {
      continue;
    }

    sockaddr_in from{};
    socklen_t from_len = sizeof(from);
    evrp::sdk::DeviceDiscoveryResponse resp{};
    const ssize_t n =
        recvfrom(fd, &resp, sizeof(resp), 0, reinterpret_cast<sockaddr*>(&from),
                 &from_len);
    if (n != static_cast<ssize_t>(sizeof(resp)) ||
        !evrp::sdk::isValidDeviceDiscoveryResponse(resp)) {
      continue;
    }
    char host[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &from.sin_addr, host, sizeof(host)) == nullptr) {
      continue;
    }
    const std::uint16_t grpc_port = ntohs(resp.grpc_port_be);
    if (grpc_port == 0) {
      continue;
    }
    std::string target = std::string(host) + ":" + std::to_string(grpc_port);
    if (seen.insert(target).second) {
      targets.push_back(std::move(target));
    }
  }

  (void)close(fd);
  sortDiscoveredTargetsPreferSameHost(&targets, local_ipv4);
  return targets;
}

class UdpDeviceDiscovererImpl final : public IUdpDeviceDiscoverer {
 public:
  explicit UdpDeviceDiscovererImpl(const ISetting& settings)
      : settings_(settings) {}

  std::vector<std::string> discoverGrpcTargets() const override {
    const int discovery_udp_port = settings_.get<int>(
        evrp::sdk::kDeviceDiscoverySettingPort, evrp::sdk::kDeviceDiscoveryUdpPort);
    evrp::sdk::DiscoveryLinkMode link_mode{};
    const std::string mode_str = settings_.get<std::string>(
        evrp::sdk::kDeviceDiscoverySettingLinkMode, std::string("multicast"));
    if (!evrp::sdk::tryParseDiscoveryLinkMode(mode_str, &link_mode)) {
      return {};
    }
    return discoverGrpcTargetsImpl(discovery_udp_port, link_mode);
  }

 private:
  const ISetting& settings_;
};

}

std::unique_ptr<IUdpDeviceDiscoverer> createUdpDeviceDiscoverer(
    const ISetting& settings) {
  return std::make_unique<UdpDeviceDiscovererImpl>(settings);
}

}
