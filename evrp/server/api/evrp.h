#pragma once

#include <memory>

#include "evrp/sdk/setting/isetting.h"

namespace grpc {
class Service;
}

namespace evrp::server {

int record(std::shared_ptr<ISetting> settings);

int replay(std::shared_ptr<ISetting> settings);

class HostControlGrpcService {
 public:
  HostControlGrpcService();
  ~HostControlGrpcService();

  HostControlGrpcService(HostControlGrpcService&&) noexcept;
  HostControlGrpcService& operator=(HostControlGrpcService&&) noexcept;

  HostControlGrpcService(const HostControlGrpcService&) = delete;
  HostControlGrpcService& operator=(const HostControlGrpcService&) = delete;

  grpc::Service* grpc_service();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}

