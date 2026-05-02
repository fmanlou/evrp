#include "evrp/sdk/listenaddress.h"

namespace evrp::sdk {

bool parseListenPort(const std::string& listenAddress, std::uint16_t* outPort) {
  if (!outPort || listenAddress.empty()) {
    return false;
  }
  std::size_t pos = listenAddress.rfind(':');
  if (listenAddress.size() >= 2 && listenAddress.front() == '[') {
    std::size_t close = listenAddress.rfind(']');
    if (close == std::string::npos || close + 2 > listenAddress.size() ||
        listenAddress[close + 1] != ':') {
      return false;
    }
    pos = close + 1;
  }
  if (pos == std::string::npos || pos + 1 >= listenAddress.size()) {
    return false;
  }
  try {
    long p = std::stol(listenAddress.substr(pos + 1));
    if (p < 1 || p > 65535) {
      return false;
    }
    *outPort = static_cast<std::uint16_t>(p);
    return true;
  } catch (...) {
    return false;
  }
}

}
