#include "evrp/sdk/sessioncheck.h"

#include <string_view>

#include "evrp/sdk/sessionmetadata.h"
#include "evrp/sdk/sessionregistry.h"

namespace evrp::session {

std::optional<std::string> sessionIdFromContext(
    const grpc::ServerContext& ctx) {
  const auto& md = ctx.client_metadata();
  const auto range = md.equal_range(kSessionMetadataKey);
  if (range.first == range.second) {
    return std::nullopt;
  }
  const grpc::string_ref& v = range.first->second;
  return std::string(v.data(), v.size());
}

grpc::Status requireBusinessSession(grpc::ServerContext* ctx,
                                    SessionRegistry& registry) {
  const auto sid = sessionIdFromContext(*ctx);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry.requireActiveBusinessCall(sv, ctx->peer());
}

}
