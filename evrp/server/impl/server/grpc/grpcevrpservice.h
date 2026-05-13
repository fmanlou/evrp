#pragma once

#include <memory>

namespace grpc {
class Service;
}

namespace evrp::server {

class Server {
 public:
  Server();
  ~Server();

  Server(Server&&) noexcept;
  Server& operator=(Server&&) noexcept;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  grpc::Service* grpc_service();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace evrp::server
