#pragma once

#include <memory>

class ISetting;

namespace evrp::device::server {

class IDiscoveryResponder {
 public:
  virtual ~IDiscoveryResponder() = default;

  virtual void start() = 0;
};

std::unique_ptr<IDiscoveryResponder> createDiscoveryResponder(
    const ISetting& settings);

}
