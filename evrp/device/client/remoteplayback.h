#pragma once

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/v1/service/playback.grpc.pb.h"

namespace evrp::device::client {
namespace api = evrp::device::api;
namespace v1 = evrp::device::v1;

class RemotePlayback final : public api::IPlayback {
 public:
  explicit RemotePlayback(std::shared_ptr<grpc::Channel> channel);

  ~RemotePlayback() override;

  RemotePlayback(const RemotePlayback&) = delete;
  RemotePlayback& operator=(const RemotePlayback&) = delete;

  bool upload(const std::vector<api::InputEvent>& events,
              api::OperationResult* result_out) override;

  bool playback(api::OperationResult* result_out,
                evrp::CountingSemaphore* progress_notify = nullptr) override;

  int playbackIndex() const override;

  bool stopPlayback() override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<v1::PlaybackService::Stub> stub_;

  std::mutex callMu_;
  std::atomic<int> reportedIndex_{-1};
};

}  // namespace evrp::device::client
