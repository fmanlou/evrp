#pragma once

#include <memory>
#include <string>

#include "evrp/device/internal/discovery/discoveryresponder.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/setting/overlaysetting.h"

class ISetting;

namespace evrp {
class Ioc;
}

namespace evrp::device::server {

class GrpcServer {
 public:
  GrpcServer(const evrp::Ioc& ioc, const ISetting& deviceSettings);
  ~GrpcServer();

  GrpcServer(const GrpcServer&) = delete;
  GrpcServer& operator=(const GrpcServer&) = delete;

  int run();

 private:
  std::string listenAddress_;
  const evrp::Ioc& ioc_;
  const ISetting& deviceSettings_;
  MemorySetting discoveryTop_;
  OverlaySetting discoveryOverlay_;
  std::unique_ptr<IDiscoveryResponder> discoveryResponder_;
};

}  // namespace evrp::device::server
