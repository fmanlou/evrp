#pragma once

#include <grpcpp/grpcpp.h>

#include <string>

namespace evrp::session {

inline constexpr char kSessionMetadataKey[] = "x-evrp-session-id";

inline void addSessionMetadata(grpc::ClientContext* ctx,
                               const std::string& sessionId) {
  if (ctx && !sessionId.empty()) {
    ctx->AddMetadata(kSessionMetadataKey, sessionId);
  }
}

}
