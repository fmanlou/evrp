#include "evrp/device/impl/server/discoveryresponder.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <thread>

#include <gflags/gflags.h>

#include "evrp/sdk/devicediscoveryprotocol.h"
#include "evrp/sdk/logger.h"

DECLARE_int32(discovery_port);

namespace evrp::device::server {

bool parseListenPort(const std::string& listen_address, std::uint16_t* out_port) {
  if (!out_port || listen_address.empty()) {
    return false;
  }
  std::size_t pos = listen_address.rfind(':');
  if (listen_address.size() >= 2 && listen_address.front() == '[') {
    std::size_t close = listen_address.rfind(']');
    if (close == std::string::npos || close + 2 > listen_address.size() ||
        listen_address[close + 1] != ':') {
      return false;
    }
    pos = close + 1;
  }
  if (pos == std::string::npos || pos + 1 >= listen_address.size()) {
    return false;
  }
  try {
    long p = std::stol(listen_address.substr(pos + 1));
    if (p < 1 || p > 65535) {
      return false;
    }
    *out_port = static_cast<std::uint16_t>(p);
    return true;
  } catch (...) {
    return false;
  }
}

namespace {

void discoveryResponderLoop(std::uint16_t grpc_listen_port, int udp_port) {
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

  logInfo("evrp-device discovery UDP on {} (gRPC port {} in replies)", udp_port,
          grpc_listen_port);

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
    resp.grpc_port_be = htons(grpc_listen_port);
    (void)sendto(fd, &resp, sizeof(resp), 0, reinterpret_cast<sockaddr*>(&from),
                 from_len);
  }
}

}  // namespace

void startDiscoveryResponder(std::uint16_t grpc_listen_port) {
  const int udp = FLAGS_discovery_port;
  if (udp < 1 || udp > 65535) {
    logError("discovery_port invalid: {}", udp);
    return;
  }
  std::thread([grpc_listen_port, udp]() {
    discoveryResponderLoop(grpc_listen_port, udp);
  }).detach();
}

}  // namespace evrp::device::server
