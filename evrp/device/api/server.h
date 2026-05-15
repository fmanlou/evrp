#pragma once

#include <memory>

class ISetting;

namespace evrp::device::api {

class IServer {
 public:
  virtual ~IServer() = default;
  virtual int run() = 0;
};

std::unique_ptr<IServer> makeServer(const ISetting& device_settings);

}
