#pragma once

#include <string>

#include "evrp/server/api/server.h"
#include "evrp/server/impl/server/grpcevrpservices.h"

class ISetting;

namespace evrp::server {

class Server final : public IServer {
 public:
  explicit Server(const ISetting& settings);
  ~Server() = default;

  int run() override;

 private:
  std::string listenAddress_;
  GrpcEvrpServices evrpGrpc_;
};

}  // namespace evrp::server
