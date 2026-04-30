#pragma once

#include <cstdint>
#include <string>

namespace evrp::device::server {

bool parseListenPort(const std::string& listen_address, std::uint16_t* out_port);
void startDiscoveryResponder(std::uint16_t grpc_listen_port);

}  // namespace evrp::device::server
