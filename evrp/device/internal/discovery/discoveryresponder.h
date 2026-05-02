#pragma once

#include <cstdint>
#include <memory>
#include <string>

class ISetting;

namespace evrp::device::server {

bool parseListenPort(const std::string& listen_address, std::uint16_t* out_port);

/// Contract for serving device discovery (e.g. UDP) alongside gRPC.
class IDiscoveryResponder {
 public:
  virtual ~IDiscoveryResponder() = default;

  /// Begin discovery for the given gRPC listen port (implementation-defined).
  virtual void start(std::uint16_t grpcListenPort) = 0;
};

/// Default discovery responder; reads kDeviceDiscoverySettingPort (int) and
/// kDeviceDiscoverySettingLinkMode (string: multicast|broadcast) from ISetting.
/// Caller must keep `settings` alive for the responder lifetime.
std::unique_ptr<IDiscoveryResponder> createDiscoveryResponder(
    const ISetting& settings);

}  // namespace evrp::device::server
