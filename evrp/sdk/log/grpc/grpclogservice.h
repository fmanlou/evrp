#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "evrp/sdk/log/asynclogbuffer.h"
#include "evrp/sdk/log/inetlogservice.h"
#include "evrp/sdk/sessionregistry.h"
#include "evrp/v1/sdk/services/log.grpc.pb.h"

namespace evrp::device::server {

class GrpcLogService final : public evrp::v1::sdk::LogService::Service,
                             public evrp::sdk::INetLogService {
 public:
  explicit GrpcLogService(evrp::session::SessionRegistry& sessions);
  ~GrpcLogService() override;

  grpc::Status StreamLogs(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<evrp::v1::sdk::LogMessage,
                               evrp::v1::sdk::LogMessage>* stream) override;

  evrp::sdk::ILogReceiveService* logReceiver() override;

  evrp::sdk::ILogSendService* logSender() override;

 private:
  class SendService final : public evrp::sdk::ILogSendService {
   public:
    explicit SendService(GrpcLogService* outer);

    logging::LogLevel getLevel() const override;

    void setLevel(logging::LogLevel level) override;

    void log(logging::LogLevel level, std::string&& msg) override;

    void flush() override;

   private:
    GrpcLogService* outer_;
  };

  class ReceiveService final : public evrp::sdk::ILogReceiveService {
   public:
    explicit ReceiveService(GrpcLogService* outer);

    logging::LogLevel getLevel() const override;

    void setLevel(logging::LogLevel level) override;

    void log(logging::LogLevel level, std::string&& msg) override;

    void flush() override;

   private:
    GrpcLogService* outer_;
  };

  void publishToAllSubscribers(logging::LogLevel level, const std::string& line);

  void ingestInbound(const evrp::v1::sdk::LogMessage& msg);

  std::atomic<logging::LogLevel> threshold_{logging::LogLevel::Info};

  std::mutex subscriberBuffersMutex_;
  std::vector<std::weak_ptr<evrp::sdk::AsyncLogBuffer>> subscriberBuffers_;

  evrp::session::SessionRegistry& sessions_;
  SendService sendService_{this};
  ReceiveService receiveService_{this};
};

}  // namespace evrp::device::server
