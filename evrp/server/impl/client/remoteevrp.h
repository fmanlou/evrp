#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>

#include "evrp/server/api/evrp.h"

namespace evrp::server {

class RemoteEvrp final : public Evrp {
 public:
  explicit RemoteEvrp(std::shared_ptr<grpc::Channel> channel);

  int record(std::shared_ptr<ISetting> settings) override;
  int replay(std::shared_ptr<ISetting> settings) override;

  bool isRecording() const override;
  bool isReplaying() const override;
  bool stopRecording() override;
  bool stopReplay() override;

 private:
  std::shared_ptr<grpc::Channel> channel_;
};

}  // namespace evrp::server
