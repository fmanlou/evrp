#pragma once

#include <string>

namespace evrp {
class Ioc;
}

namespace evrp::device::api {

int runDeviceServer(const std::string& listen_address, const evrp::Ioc& ioc);

}  // namespace evrp::device::api
