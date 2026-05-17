#include "evrp/device/impl/server/grpc/grpclogservice.h"

#include <algorithm>

#include "evrp/sdk/sessioncheck.h"
#include "log/spdlog/spdlogger.h"

namespace evrp::device::server {

GrpcLogService::GrpcLogService(evrp::session::SessionRegistry& sessions)
    : logging::ILogService("GrpcLogService"), sessions_(sessions) {}

GrpcLogService::~GrpcLogService() {
  std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
  subscriberBuffers_.clear();
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

logging::LogLevel GrpcLogService::getLevel() const {
  return threshold_;
}

void GrpcLogService::setLevel(logging::LogLevel level) {
  threshold_ = level;
}

void GrpcLogService::log(logging::LogLevel level, std::string&& msg) {
  if (level < threshold_) {
    return;
  }
  publishToAllSubscribers(level, msg);
}

void GrpcLogService::flush() {}

grpc::Status GrpcLogService::SubscribeLogs(
    grpc::ServerContext* context,
    const google::protobuf::Empty* ,
    grpc::ServerWriter<evrp::v1::sdk::LogMessage>* writer) {
  if (grpc::Status st = evrp::session::requireBusinessSession(context, sessions_);
      !st.ok()) {
    return st;
  }

  auto buffer = std::make_shared<evrp::sdk::AsyncLogBuffer>();
  {
    std::lock_guard<std::mutex> lock(subscriberBuffersMutex_);
    subscriberBuffers_.push_back(buffer);
  }

  grpc::Status status = grpc::Status::OK;

  for (;;) {
    if (context->IsCancelled()) {
      status = grpc::Status(grpc::StatusCode::CANCELLED, "cancelled");
      break;
    }
    std::optional<evrp::sdk::LogMessage> line =
        buffer->waitPopFor(std::chrono::milliseconds(200));
    if (line.has_value()) {
      evrp::v1::sdk::LogMessage msg;
      msg.set_spdlog_level(
          static_cast<int>(logging::levelToSpdlog(line->level)));
      msg.set_formatted_line(std::move(line->text));
      if (!writer->Write(msg)) {
        status = grpc::Status(grpc::StatusCode::UNKNOWN, "write failed");
        break;
      }
    }
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
