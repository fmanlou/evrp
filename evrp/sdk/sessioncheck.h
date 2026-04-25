#pragma once

#include <grpcpp/grpcpp.h>

#include <optional>
#include <string>

namespace evrp::session {

class SessionRegistry;

std::optional<std::string> sessionIdFromContext(
    const grpc::ServerContext& ctx);

grpc::Status requireBusinessSession(grpc::ServerContext* ctx,
                                    SessionRegistry& registry);

}
