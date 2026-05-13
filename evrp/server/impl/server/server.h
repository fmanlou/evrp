#pragma once

#include <memory>

#include "evrp/v1/server/service/evrp.grpc.pb.h"

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
  std::unique_ptr<evrp::v1::server::EvrpService::Service> service_;
};

}  // namespace evrp::server
