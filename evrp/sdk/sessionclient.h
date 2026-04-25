#pragma once

#include <memory>
#include <string>

namespace grpc {
class Channel;
}

namespace evrp::sdk {

struct SessionInfo {
  std::string sessionId;
  int leaseTimeoutMs = 0;
};

std::shared_ptr<grpc::Channel> makeGrpcClientChannel(
    const std::string& targetHostPort);

bool sessionConnect(const std::shared_ptr<grpc::Channel>& channel,
                    SessionInfo* out);
bool sessionHeartbeat(const std::shared_ptr<grpc::Channel>& channel,
                      const std::string& sessionId);
bool sessionDisconnect(const std::shared_ptr<grpc::Channel>& channel,
                       const std::string& sessionId);

}
