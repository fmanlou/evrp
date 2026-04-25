#pragma once

#include <memory>
#include <string>

namespace evrp {
class Ioc;
}

namespace evrp::device::api {

class IServer {
 public:
  virtual ~IServer() = default;
  virtual int run() = 0;
};

std::unique_ptr<IServer> makeServer(const std::string& listen_address,
                                    const evrp::Ioc& ioc);

}
