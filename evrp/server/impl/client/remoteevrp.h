#pragma once

#include <grpcpp/grpcpp.h>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "evrp/server/api/evrp.h"

namespace evrp::server {

class RemoteEvrp final : public Evrp {
 public:
  RemoteEvrp(std::shared_ptr<grpc::Channel> channel, std::string sessionId,
             int leaseTimeoutMs);

  ~RemoteEvrp() override;

  RemoteEvrp(const RemoteEvrp&) = delete;
  RemoteEvrp& operator=(const RemoteEvrp&) = delete;

  int record(std::shared_ptr<ISetting> settings) override;
  int replay(std::shared_ptr<ISetting> settings) override;

  bool isRecording() const override;
  bool isReplaying() const override;
  bool stopRecording() override;
  bool stopReplay() override;

 private:
  void heartbeatLoop();

  std::shared_ptr<grpc::Channel> channel_;
  std::string sessionId_;
  int leaseTimeoutMs_;
  std::atomic<bool> heartbeatStop_{false};
  std::thread heartbeatThread_;
};

}  // namespace evrp::server
