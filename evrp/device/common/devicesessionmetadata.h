#pragma once

#include <grpcpp/grpcpp.h>

#include <string>

namespace evrp::device {

// gRPC 客户端 metadata 键；Connect 成功后对所有业务 RPC 与 Heartbeat/Disconnect 附加此字段。
inline constexpr char kDeviceSessionMetadataKey[] = "x-evrp-device-session-id";

inline void addDeviceSessionMetadata(grpc::ClientContext* ctx,
                                     const std::string& sessionId) {
  if (ctx && !sessionId.empty()) {
    ctx->AddMetadata(kDeviceSessionMetadataKey, sessionId);
  }
}

}  // namespace evrp::device
