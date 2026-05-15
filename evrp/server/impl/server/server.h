#pragma once

#include <memory>
#include <string>

#include "evrp/server/api/server.h"
#include "evrp/v1/server/service/evrp.grpc.pb.h"

class ISetting;

namespace evrp::server {

class Server final : public IServer {
 public:
  explicit Server(const ISetting& settings);
  ~Server() override;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  int run() override;

 private:
  std::string listenAddress_;
  std::unique_ptr<evrp::v1::server::EvrpService::Service> evrpService_;
};

}  // namespace evrp::server
