#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include <grpcpp/support/status.h>

namespace evrp::device::server {

// 业务层会话（与 gRPC TCP 连接无关）：Connect 创建、Heartbeat 续期、超时或 Disconnect 清除。
class DeviceSessionRegistry {
 public:
  explicit DeviceSessionRegistry(int leaseTimeoutMs);

  DeviceSessionRegistry(const DeviceSessionRegistry&) = delete;
  DeviceSessionRegistry& operator=(const DeviceSessionRegistry&) = delete;

  std::string connect(std::string_view peer);
  grpc::Status heartbeat(std::string_view sessionId, std::string_view peer);
  grpc::Status disconnect(std::string_view sessionId, std::string_view peer);
  grpc::Status requireActiveBusinessCall(std::string_view sessionId,
                                         std::string_view peer);

  int leaseTimeoutMs() const { return leaseTimeoutMs_; }

  void sweepExpiredForLogging();

 private:
  struct Record {
    std::string peer;
    std::chrono::steady_clock::time_point lastHeartbeat;
  };

  int leaseTimeoutMs_;
  std::mutex mutex_;
  std::unordered_map<std::string, Record> sessions_;
};

}  // namespace evrp::device::server
