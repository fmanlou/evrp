#pragma once

#include <cstdlib>
#include <string>
#include <vector>

namespace evrp::device::api {

inline bool useUdpDeviceDiscovery(const std::string& device_flag) {
  return device_flag.empty();
}

std::vector<std::string> discoverDeviceGrpcTargetsViaUdp(int discovery_udp_port);

}  // namespace evrp::device::api
