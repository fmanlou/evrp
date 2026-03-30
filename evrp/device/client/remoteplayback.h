#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "evrp/device/api/playback.h"
#include "evrp/device/v1/service/playback.grpc.pb.h"

namespace evrp::device::client {

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

  int playback_index() const override;

  bool stop_playback() override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<evrp::device::v1::PlaybackService::Stub> stub_;

  std::mutex call_mu_;
  std::atomic<int> reported_index_{-1};
};

}  // namespace evrp::device::client
