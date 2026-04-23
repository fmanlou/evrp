#pragma once

#include <grpcpp/grpcpp.h>

#include <string>

namespace evrp::device {

inline constexpr char kDeviceSessionMetadataKey[] = "x-evrp-device-session-id";

inline void addDeviceSessionMetadata(grpc::ClientContext* ctx,
                                     const std::string& sessionId) {
  if (ctx && !sessionId.empty()) {
    ctx->AddMetadata(kDeviceSessionMetadataKey, sessionId);
  }
}

}
