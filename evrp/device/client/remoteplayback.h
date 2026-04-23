#pragma once

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/v1/service/playback.grpc.pb.h"

#include <string>

namespace evrp::device::client {

class RemotePlayback final : public api::IPlayback {
 public:
  RemotePlayback(std::shared_ptr<grpc::Channel> channel,
                 std::string deviceSessionId);

  ~RemotePlayback() override;

  RemotePlayback(const RemotePlayback&) = delete;
  RemotePlayback& operator=(const RemotePlayback&) = delete;

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* resultOut) override;

  bool playback(api::OperationResult* resultOut,
                evrp::CountingSemaphore* progressNotify = nullptr) override;

  int playbackIndex() const override;

  bool stopPlayback() override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<v1::PlaybackService::Stub> stub_;
  std::string deviceSessionId_;

  std::mutex callMu_;
  std::atomic<int> reportedIndex_{-1};
};

}  // namespace evrp::device::client
