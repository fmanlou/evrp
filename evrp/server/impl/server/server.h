#pragma once

#include <asio/io_context.hpp>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "evrp/server/api/server.h"
#include "evrp/server/impl/server/localevrp.h"
#include "evrp/server/impl/server/posted/postedevrp.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/device/impl/server/grpc/grpcsessionservice.h"
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
  asio::io_context ioContext_;
  std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
      workGuard_;
  evrp::session::SessionRegistry clientSessionRegistry_;
  evrp::device::server::GrpcSessionService clientSessionService_;
  LocalEvrp localEvrp_;
  PostedEvrp postedEvrp_;
  std::thread worker_;
  std::unique_ptr<evrp::v1::server::EvrpService::Service> evrpService_;
};

}  // namespace evrp::server
