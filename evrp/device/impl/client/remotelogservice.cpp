#include "evrp/device/impl/client/remotelogservice.h"

#include <chrono>
#include <thread>

#include <google/protobuf/empty.pb.h>
#include <spdlog/common.h>

#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/sessionmetadata.h"
#include "log/spdlog/spdlogger.h"

namespace evrp::device::client {

RemoteLogService::RemoteLogService(std::shared_ptr<grpc::Channel> channel,
                                   std::string deviceSessionId)
    : stub_(evrp::v1::sdk::LogService::NewStub(std::move(channel))),
      deviceSessionId_(std::move(deviceSessionId)) {}

RemoteLogService::~RemoteLogService() = default;

void RemoteLogService::forwardUntil(std::atomic<bool>* stopFlag) {
  if (!stub_ || !stopFlag || deviceSessionId_.empty() || !logService) {
    return;
  }

  grpc::ClientContext ctx;
  evrp::session::addSessionMetadata(&ctx, deviceSessionId_);

  std::thread cancelOnStop([stopFlag, &ctx]() {
    while (!stopFlag->load(std::memory_order_acquire)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    ctx.TryCancel();
  });

  google::protobuf::Empty req;
  std::unique_ptr<grpc::ClientReader<evrp::v1::sdk::LogMessage>> reader =
      stub_->SubscribeLogs(&ctx, req);

  evrp::v1::sdk::LogMessage msg;
  while (!stopFlag->load(std::memory_order_acquire) && reader->Read(&msg)) {
    const logging::LogLevel lvl = logging::spdlogToLevel(
        static_cast<spdlog::level::level_enum>(msg.spdlog_level()));
    logService->log(lvl, std::string("[evrp-device] ") + msg.formatted_line());
  }

  cancelOnStop.join();
  (void)reader->Finish();
}

}  // namespace evrp::device::client
