#include "evrp/sdk/log/grpc/remotelogservice.h"

#include <chrono>
#include <thread>

#include <spdlog/common.h>

#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/sessionmetadata.h"
#include "log/spdlog/spdlogger.h"

namespace evrp::device::client {

RemoteLogService::SendService::SendService(RemoteLogService* outer)
    : outer_(outer) {}

logging::LogLevel RemoteLogService::SendService::getLevel() const {
  return logService ? logService->getLevel() : logging::LogLevel::Info;
}

void RemoteLogService::SendService::setLevel(logging::LogLevel level) {
  if (logService) {
    logService->setLevel(level);
  }
}

void RemoteLogService::SendService::log(logging::LogLevel level,
                                        std::string&& msg) {
  outer_->writeOutbound(level, msg);
}

void RemoteLogService::SendService::flush() {}

RemoteLogService::ReceiveService::ReceiveService(RemoteLogService* outer)
    : outer_(outer) {}

logging::LogLevel RemoteLogService::ReceiveService::getLevel() const {
  return logService ? logService->getLevel() : logging::LogLevel::Info;
}

void RemoteLogService::ReceiveService::setLevel(logging::LogLevel level) {
  if (logService) {
    logService->setLevel(level);
  }
}

void RemoteLogService::ReceiveService::log(logging::LogLevel level,
                                           std::string&& msg) {
  if (logService) {
    logService->log(level, std::move(msg));
  }
}

void RemoteLogService::ReceiveService::flush() {
  if (logService) {
    logService->flush();
  }
}

RemoteLogService::RemoteLogService(std::shared_ptr<grpc::Channel> channel,
                                   std::string deviceSessionId)
    : stub_(evrp::v1::sdk::LogService::NewStub(std::move(channel))),
      deviceSessionId_(std::move(deviceSessionId)) {}

RemoteLogService::~RemoteLogService() = default;

evrp::sdk::ILogReceiveService* RemoteLogService::logReceiver() {
  return &receiveService_;
}

evrp::sdk::ILogSendService* RemoteLogService::logSender() {
  return &sendService_;
}

void RemoteLogService::ingestInbound(const evrp::v1::sdk::LogMessage& msg) {
  const logging::LogLevel level = logging::spdlogToLevel(
      static_cast<spdlog::level::level_enum>(msg.spdlog_level()));
  receiveService_.log(level,
                      std::string("[evrp-device] ") + msg.formatted_line());
}

void RemoteLogService::writeOutbound(logging::LogLevel level,
                                     const std::string& line) {
  std::lock_guard<std::mutex> lock(streamMutex_);
  if (!activeStream_) {
    return;
  }
  evrp::v1::sdk::LogMessage outbound;
  outbound.set_spdlog_level(static_cast<int>(logging::levelToSpdlog(level)));
  outbound.set_formatted_line(line);
  (void)activeStream_->Write(outbound);
}

void RemoteLogService::forwardUntil(std::atomic<bool>* stopFlag) {
  if (!stub_ || !stopFlag || deviceSessionId_.empty()) {
    return;
  }

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);

  std::unique_ptr<
      grpc::ClientReaderWriter<evrp::v1::sdk::LogMessage, evrp::v1::sdk::LogMessage>>
      stream = stub_->StreamLogs(&ctx);

  {
    std::lock_guard<std::mutex> lock(streamMutex_);
    activeStream_ = stream.get();
  }

  std::thread cancelOnStop([stopFlag, &ctx]() {
    while (!stopFlag->load(std::memory_order_acquire)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    ctx.TryCancel();
  });

  evrp::v1::sdk::LogMessage inbound;
  while (!stopFlag->load(std::memory_order_acquire) && stream->Read(&inbound)) {
    ingestInbound(inbound);
  }

  cancelOnStop.join();

  {
    std::lock_guard<std::mutex> lock(streamMutex_);
    activeStream_ = nullptr;
  }

  stream->WritesDone();
  (void)stream->Finish();
}

}  // namespace evrp::device::client
