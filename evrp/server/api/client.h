#pragma once

#include <memory>
#include <string>

namespace evrp::server {

class Evrp;

class Client {
 public:
  virtual ~Client() = default;

  virtual Evrp* evrp() const = 0;
  virtual const std::string& serverAddress() const = 0;
};

std::unique_ptr<Client> makeClient();

/** Connect to evrp-server `EvrpService` at HOST:PORT or unix:/PATH. */
std::unique_ptr<Client> makeClient(std::string evrp_service_address);

}  // namespace evrp::server
