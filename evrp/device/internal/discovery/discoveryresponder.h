#pragma once

#include <cstdint>
#include <string>

#include "evrp/device/internal/discovery/devicediscoveryprotocol.h"

class ISetting;

namespace evrp::device::server {

bool parseListenPort(const std::string& listen_address, std::uint16_t* out_port);

/// UDP discovery responder; reads kDeviceDiscoverySettingPort (int) and
/// kDeviceDiscoverySettingLinkMode (string: multicast|broadcast) from ISetting.
/// Caller must keep `settings` alive for the server lifetime.
class DiscoveryResponder {
 public:
  explicit DiscoveryResponder(const ISetting& settings);

  /// Spawns a detached thread that serves discovery until process exit.
  void start(std::uint16_t grpc_listen_port);

 private:
  const ISetting& settings_;
};

}  // namespace evrp::device::server
