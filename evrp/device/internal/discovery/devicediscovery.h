#pragma once

#include <memory>
#include <string>
#include <vector>

class ISetting;

namespace evrp::device::api {

inline bool useUdpDeviceDiscovery(const std::string& device_flag) {
  return device_flag.empty();
}

/// Contract for discovering device gRPC endpoints (e.g. via UDP).
class IUdpDeviceDiscoverer {
 public:
  virtual ~IUdpDeviceDiscoverer() = default;

  virtual std::vector<std::string> discoverGrpcTargets() const = 0;
};

/// Default UDP discovery client; reads kDeviceDiscoverySettingPort (int) and
/// kDeviceDiscoverySettingLinkMode (string) from ISetting.
/// Caller must keep `settings` alive for the discoverer lifetime.
std::unique_ptr<IUdpDeviceDiscoverer> createUdpDeviceDiscoverer(
    const ISetting& settings);

}  // namespace evrp::device::api
