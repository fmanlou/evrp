#pragma once

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

namespace evrp::device::api {
class IClient;
}

namespace evrp::device::impl {

/** If \a client is the concrete device client implementation, fills channel and
 *  session id used for device RPCs (e.g. log streaming). Otherwise returns false. */
bool tryExportSessionForLogForwarding(
    const api::IClient* client,
    std::shared_ptr<grpc::Channel>* channel,
    std::string* session_id);

}  // namespace evrp::device::impl
