#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include <google/protobuf/empty.pb.h>

#include "evrp/sdk/log/asynclogbuffer.h"
#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/v1/sdk/services/log.grpc.pb.h"

namespace evrp::device::server {

class GrpcLogService final : public evrp::v1::sdk::LogService::Service,
                             public logging::ILogService {
 public:
  explicit GrpcLogService(evrp::session::SessionRegistry& sessions);
  ~GrpcLogService() override;

  grpc::Status SubscribeLogs(
      grpc::ServerContext* context,
      const google::protobuf::Empty* request,
      grpc::ServerWriter<evrp::v1::sdk::LogMessage>* writer) override;

  logging::LogLevel getLevel() const override;

  void setLevel(logging::LogLevel level) override;

  void log(logging::LogLevel level, std::string&& msg) override;

  void flush() override;

 private:
  void publishToAllSubscribers(logging::LogLevel level, const std::string& line);

  std::atomic<logging::LogLevel> threshold_{logging::LogLevel::Info};

  std::mutex subscriberBuffersMutex_;
  std::vector<std::weak_ptr<evrp::sdk::AsyncLogBuffer>> subscriberBuffers_;

  evrp::session::SessionRegistry& sessions_;
};

}  // namespace evrp::device::server
