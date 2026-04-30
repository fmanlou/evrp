#include "evrp/device/impl/client/devicediscovery.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
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

#include "evrp/sdk/devicediscoveryprotocol.h"

DEFINE_int32(
    discovery_port, evrp::sdk::kDeviceDiscoveryUdpPort,
    "UDP port for LAN discovery (empty --device on client; evrp-device must use the same).");

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

}  // namespace

std::vector<std::string> discoverDeviceGrpcTargetsViaUdp(int discovery_udp_port) {
  std::vector<std::string> targets;
  if (discovery_udp_port < 1 || discovery_udp_port > 65535) {
    return targets;
  }

  const std::unordered_set<std::string> local_ipv4 = collectLocalIpv4Addresses();

  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    return targets;
  }
  int broadcast = 1;
  (void)setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

  const std::uint16_t udp_be = htons(static_cast<std::uint16_t>(discovery_udp_port));

  sockaddr_in loop{};
  loop.sin_family = AF_INET;
  loop.sin_port = udp_be;
  loop.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  (void)sendDiscoveryProbe(
      fd, reinterpret_cast<sockaddr*>(&loop), sizeof(loop));

  sockaddr_in global_bcast{};
  global_bcast.sin_family = AF_INET;
  global_bcast.sin_port = udp_be;
  global_bcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  (void)sendDiscoveryProbe(
      fd, reinterpret_cast<sockaddr*>(&global_bcast), sizeof(global_bcast));

  ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) == 0) {
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

  std::set<std::string> seen;
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(800);

  while (std::chrono::steady_clock::now() < deadline) {
    const auto remain = std::chrono::duration_cast<std::chrono::milliseconds>(
                            deadline - std::chrono::steady_clock::now())
                            .count();
    if (remain <= 0) {
      break;
    }
    timeval tv{};
    tv.tv_sec = static_cast<long>(remain / 1000);
    tv.tv_usec = static_cast<long>((remain % 1000) * 1000);

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

}  // namespace evrp::device::api
