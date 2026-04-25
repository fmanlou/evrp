#include "evrp/sdk/sessionregistry.h"

#include <random>
#include <sstream>

#include <gflags/gflags.h>

#include "evrp/sdk/sessionmetadata.h"
#include "logger.h"

DEFINE_int32(
    session_lease_ms,
    3000,
    "Business session expires if Heartbeat is not received within this "
    "interval (ms).");

namespace evrp::session {
namespace {

std::string randomSessionId() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::ostringstream ss;
  ss << std::hex << gen() << gen();
  return ss.str();
}

}  

SessionRegistry::SessionRegistry(int leaseTimeoutMs) {
  int ms = leaseTimeoutMs > 0 ? leaseTimeoutMs : FLAGS_session_lease_ms;
  if (ms <= 0) {
    ms = 3000;
  }
  leaseTimeoutMs_ = ms;
}

std::string SessionRegistry::connect(std::string_view peer) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::string id;
  do {
    id = randomSessionId();
  } while (sessions_.count(id));
  Record r;
  r.peer = std::string(peer);
  r.lastHeartbeat = std::chrono::steady_clock::now();
  sessions_.emplace(id, std::move(r));
  logInfo("evrp: business session online peer={} session={}", peer, id);
  return id;
}

grpc::Status SessionRegistry::heartbeat(std::string_view sessionId,
                                        std::string_view peer) {
  if (sessionId.empty()) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        std::string("metadata ") + kSessionMetadataKey +
            " required for Heartbeat");
  }
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = sessions_.find(std::string(sessionId));
  if (it == sessions_.end()) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "unknown or expired session; call Connect again");
  }
  if (it->second.peer != peer) {
    return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                        "session peer mismatch");
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - it->second.lastHeartbeat > std::chrono::milliseconds(leaseTimeoutMs_)) {
    sessions_.erase(it);
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        "session lease expired before Heartbeat; call Connect again");
  }
  it->second.lastHeartbeat = now;
  return grpc::Status::OK;
}

grpc::Status SessionRegistry::disconnect(std::string_view sessionId,
                                         std::string_view peer) {
  if (sessionId.empty()) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        std::string("metadata ") + kSessionMetadataKey +
            " required for Disconnect");
  }
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = sessions_.find(std::string(sessionId));
  if (it == sessions_.end()) {
    return grpc::Status::OK;
  }
  if (it->second.peer != peer) {
    return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                        "session peer mismatch");
  }
  logInfo("evrp: business session offline peer={} session={}",
          it->second.peer,
          sessionId);
  sessions_.erase(it);
  return grpc::Status::OK;
}

grpc::Status SessionRegistry::requireActiveBusinessCall(
    std::string_view sessionId, std::string_view peer) {
  if (sessionId.empty()) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        std::string("business session required: call session Connect then "
                    "attach metadata ") +
            kSessionMetadataKey + " on each RPC");
  }
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = sessions_.find(std::string(sessionId));
  if (it == sessions_.end()) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "unknown session; call Connect or session was evicted");
  }
  if (it->second.peer != peer) {
    return grpc::Status(grpc::StatusCode::PERMISSION_DENIED,
                        "session peer mismatch");
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - it->second.lastHeartbeat > std::chrono::milliseconds(leaseTimeoutMs_)) {
    sessions_.erase(it);
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        "session lease expired; call Connect and keep Heartbeat within "
        "lease_timeout_ms");
  }
  return grpc::Status::OK;
}

void SessionRegistry::sweepExpiredForLogging() {
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto it = sessions_.begin(); it != sessions_.end();) {
    if (now - it->second.lastHeartbeat >
        std::chrono::milliseconds(leaseTimeoutMs_)) {
      logWarn("evrp: business session timed out peer={} session={}",
              it->second.peer,
              it->first);
      it = sessions_.erase(it);
    } else {
      ++it;
    }
  }
}

}
