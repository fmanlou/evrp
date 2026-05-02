#pragma once

#include <memory>
#include <string>
#include <vector>

class ISetting;

namespace evrp::device::api {

inline bool useUdpDeviceDiscovery(const std::string& device_flag) {
  return device_flag.empty();
}

class IUdpDeviceDiscoverer {
 public:
  virtual ~IUdpDeviceDiscoverer() = default;

  virtual std::vector<std::string> discoverGrpcTargets() const = 0;
};

std::unique_ptr<IUdpDeviceDiscoverer> createUdpDeviceDiscoverer(
    const ISetting& settings);

}
