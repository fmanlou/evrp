#pragma once

#include <cstdint>
#include <memory>
#include <string>

class ISetting;

namespace evrp::device::server {

bool parseListenPort(const std::string& listen_address, std::uint16_t* out_port);

class IDiscoveryResponder {
 public:
  virtual ~IDiscoveryResponder() = default;

  virtual void start(std::uint16_t grpcListenPort) = 0;
};

std::unique_ptr<IDiscoveryResponder> createDiscoveryResponder(
    const ISetting& settings);

}
