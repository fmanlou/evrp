#pragma once

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <string>

#include "evrp/v1/sdk/services/log.grpc.pb.h"

namespace evrp::device::client {

class RemoteLogService {
 public:
  RemoteLogService(std::shared_ptr<grpc::Channel> channel,
                   std::string deviceSessionId);

  ~RemoteLogService();

  RemoteLogService(const RemoteLogService&) = delete;
  RemoteLogService& operator=(const RemoteLogService&) = delete;

  void forwardUntil(std::atomic<bool>* stopFlag);

 private:
  std::unique_ptr<evrp::v1::sdk::LogService::Stub> stub_;
  std::string deviceSessionId_;
};

}
