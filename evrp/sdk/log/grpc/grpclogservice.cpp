#include "evrp/sdk/log/grpc/grpclogservice.h"

#include <algorithm>
#include <thread>

#include <spdlog/common.h>

#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/sessioncheck.h"
#include "log/spdlog/spdlogger.h"

namespace evrp::device::server {

GrpcLogService::SendService::SendService(GrpcLogService* outer) : outer_(outer) {}

logging::LogLevel GrpcLogService::SendService::getLevel() const {
  return outer_->threshold_;
}

void GrpcLogService::SendService::setLevel(logging::LogLevel level) {
  outer_->threshold_ = level;
}

void GrpcLogService::SendService::log(logging::LogLevel level, std::string&& msg) {
  if (level < outer_->threshold_) {
    return;
  }
  outer_->publishToAllSubscribers(level, msg);
}

void GrpcLogService::SendService::flush() {}

GrpcLogService::ReceiveService::ReceiveService(GrpcLogService* outer)
    : outer_(outer) {}

logging::LogLevel GrpcLogService::ReceiveService::getLevel() const {
  return outer_->threshold_;
}

void GrpcLogService::ReceiveService::setLevel(logging::LogLevel level) {
  outer_->threshold_ = level;
}

void GrpcLogService::ReceiveService::log(logging::LogLevel level,
                                         std::string&& msg) {
  if (level < outer_->threshold_ || !logService) {
    return;
  }
  logService->log(level, std::move(msg));
}

void GrpcLogService::ReceiveService::flush() {
  if (logService) {
    logService->flush();
  }
}

GrpcLogService::GrpcLogService(evrp::session::SessionRegistry& sessions)
    : sessions_(sessions) {}

GrpcLogService::~GrpcLogService() {
  std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
  subscriberBuffers_.clear();
}

evrp::sdk::ILogReceiveService* GrpcLogService::logReceiver() {
  return &receiveService_;
}

evrp::sdk::ILogSendService* GrpcLogService::logSender() {
  return &sendService_;
}

void GrpcLogService::publishToAllSubscribers(logging::LogLevel level,
                                             const std::string& line) {
  std::vector<std::shared_ptr<evrp::sdk::AsyncLogBuffer>> snap;
  {
    std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
    for (auto it = subscriberBuffers_.begin(); it != subscriberBuffers_.end();) {
      if (auto buf = it->lock()) {
        snap.push_back(std::move(buf));
        ++it;
      } else {
        it = subscriberBuffers_.erase(it);
      }
    }
  }
  for (const auto& buf : snap) {
    buf->push(level, line);
  }
}

void GrpcLogService::ingestInbound(const evrp::v1::sdk::LogMessage& msg) {
  const logging::LogLevel level = logging::spdlogToLevel(
      static_cast<spdlog::level::level_enum>(msg.spdlog_level()));
  receiveService_.log(level,
                      std::string("[remote] ") + msg.formatted_line());
}

grpc::Status GrpcLogService::StreamLogs(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<evrp::v1::sdk::LogMessage,
                             evrp::v1::sdk::LogMessage>* stream) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_);
      !st.ok()) {
    return st;
  }

  auto buffer = std::make_shared<evrp::sdk::AsyncLogBuffer>();
  {
    std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
    subscriberBuffers_.push_back(buffer);
  }

  std::atomic<bool> readDone{false};
  std::thread reader([this, stream, context, &readDone]() {
    evrp::v1::sdk::LogMessage inbound;
    while (stream->Read(&inbound)) {
      if (context->IsCancelled()) {
        break;
      }
      ingestInbound(inbound);
    }
    readDone.store(true, std::memory_order_release);
  });

  grpc::Status status = grpc::Status::OK;
  while (!context->IsCancelled() &&
         !readDone.load(std::memory_order_acquire)) {
    std::optional<evrp::sdk::LogMessage> line =
        buffer->waitPopFor(std::chrono::milliseconds(200));
    if (line.has_value()) {
      evrp::v1::sdk::LogMessage outbound;
      outbound.set_spdlog_level(
          static_cast<int>(logging::levelToSpdlog(line->level)));
      outbound.set_formatted_line(std::move(line->text));
      if (!stream->Write(outbound)) {
        status = grpc::Status(grpc::StatusCode::UNKNOWN, "write failed");
        break;
      }
    }
  }

  if (context->IsCancelled()) {
    status = grpc::Status(grpc::StatusCode::CANCELLED, "cancelled");
  }

  readDone.store(true, std::memory_order_release);
  if (reader.joinable()) {
    reader.join();
  }

  {
    std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
    subscriberBuffers_.erase(
        std::remove_if(
            subscriberBuffers_.begin(), subscriberBuffers_.end(),
            [&](const std::weak_ptr<evrp::sdk::AsyncLogBuffer>& w) {
              return w.lock() == buffer || w.expired();
            }),
        subscriberBuffers_.end());
  }

  return status;
}

}  // namespace evrp::device::server
