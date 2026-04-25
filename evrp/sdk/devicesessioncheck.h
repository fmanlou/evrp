#pragma once

#include <grpcpp/grpcpp.h>

#include <optional>
#include <string>

namespace evrp::device::server {

class DeviceSessionRegistry;

std::optional<std::string> deviceSessionIdFromContext(
    const grpc::ServerContext& ctx);

grpc::Status requireDeviceBusinessSession(grpc::ServerContext* ctx,
                                          DeviceSessionRegistry& registry);

}
