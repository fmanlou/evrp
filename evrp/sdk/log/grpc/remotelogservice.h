#pragma once

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "evrp/sdk/log/inetlogservice.h"
#include "evrp/v1/sdk/services/log.grpc.pb.h"

namespace evrp::device::client {

class RemoteLogService final : public evrp::sdk::INetLogService {
 public:
  RemoteLogService(std::shared_ptr<grpc::Channel> channel,
                   std::string deviceSessionId);

  ~RemoteLogService() override;

  RemoteLogService(const RemoteLogService&) = delete;
  RemoteLogService& operator=(const RemoteLogService&) = delete;

  evrp::sdk::ILogReceiveService* logReceiver() override;

  evrp::sdk::ILogSendService* logSender() override;

  void forwardUntil(std::atomic<bool>* stopFlag) override;

 private:
  class SendService final : public evrp::sdk::ILogSendService {
   public:
    explicit SendService(RemoteLogService* outer);

    logging::LogLevel getLevel() const override;

    void setLevel(logging::LogLevel level) override;

    void log(logging::LogLevel level, std::string&& msg) override;

    void flush() override;

   private:
    RemoteLogService* outer_;
  };

  class ReceiveService final : public evrp::sdk::ILogReceiveService {
   public:
    explicit ReceiveService(RemoteLogService* outer);

    logging::LogLevel getLevel() const override;

    void setLevel(logging::LogLevel level) override;

    void log(logging::LogLevel level, std::string&& msg) override;

    void flush() override;

   private:
    RemoteLogService* outer_;
  };

  void ingestInbound(const evrp::v1::sdk::LogMessage& msg);

  void writeOutbound(logging::LogLevel level, const std::string& line);

  std::unique_ptr<evrp::v1::sdk::LogService::Stub> stub_;
  std::string deviceSessionId_;

  std::mutex streamMutex_;
  grpc::ClientReaderWriter<evrp::v1::sdk::LogMessage, evrp::v1::sdk::LogMessage>*
      activeStream_{nullptr};
  SendService sendService_{this};
  ReceiveService receiveService_{this};
};

}
