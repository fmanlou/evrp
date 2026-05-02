#pragma once

#include <string>
#include <vector>

#include "evrp/device/internal/discovery/devicediscoveryprotocol.h"

class ISetting;

namespace evrp::device::api {

inline bool useUdpDeviceDiscovery(const std::string& device_flag) {
  return device_flag.empty();
}

/// UDP discovery client; reads kDeviceDiscoverySettingPort (int) and
/// kDeviceDiscoverySettingLinkMode (string) from ISetting.
class UdpDeviceDiscoverer {
 public:
  explicit UdpDeviceDiscoverer(const ISetting& settings);

  std::vector<std::string> discoverGrpcTargets() const;

 private:
  const ISetting& settings_;
};

}  // namespace evrp::device::api
