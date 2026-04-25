#include "evrp/device/impl/server/devicesessioncheck.h"

#include <string_view>

#include "evrp/device/common/devicesessionmetadata.h"
#include "evrp/device/impl/server/devicesessionregistry.h"

namespace evrp::device::server {

std::optional<std::string> deviceSessionIdFromContext(
    const grpc::ServerContext& ctx) {
  const auto& md = ctx.client_metadata();
  const auto range = md.equal_range(evrp::device::kDeviceSessionMetadataKey);
  if (range.first == range.second) {
    return std::nullopt;
  }
  const grpc::string_ref& v = range.first->second;
  return std::string(v.data(), v.size());
}

grpc::Status requireDeviceBusinessSession(grpc::ServerContext* ctx,
                                          DeviceSessionRegistry& registry) {
  const auto sid = deviceSessionIdFromContext(*ctx);
  const std::string_view sv =
      sid ? std::string_view(*sid) : std::string_view();
  return registry.requireActiveBusinessCall(sv, ctx->peer());
}

}
