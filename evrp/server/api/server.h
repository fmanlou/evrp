#pragma once

#include <memory>

class ISetting;

namespace evrp::server {

class IServer {
 public:
  virtual ~IServer() = default;
  virtual int run() = 0;
};

std::unique_ptr<IServer> makeServer(const ISetting& settings);

}  // namespace evrp::server
