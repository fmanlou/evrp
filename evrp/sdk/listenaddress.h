#pragma once

#include <cstdint>
#include <string>

namespace evrp::sdk {

bool parseListenPort(const std::string& listenAddress, std::uint16_t* outPort);

}
