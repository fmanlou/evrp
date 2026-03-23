// evrp-device：设备端进程入口。仅依赖 api 层（server.h / IDeviceHost），不包含传输实现头文件。

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "evrp/device/api/server.h"
#include "evrp/device/stub_device_host.h"

namespace {

const char* kDefaultListen = "127.0.0.1:50051";

void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [--listen ADDRESS]\n"
            << "  ADDRESS  listen address (default " << kDefaultListen << ")\n";
}

std::string ParseListen(int argc, char** argv) {
  std::string addr = kDefaultListen;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--listen") == 0 && i + 1 < argc) {
      addr = argv[++i];
    } else if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
      PrintUsage(argv[0]);
      std::exit(0);
    } else {
      std::cerr << "Unknown argument: " << argv[i] << "\n";
      PrintUsage(argv[0]);
      std::exit(1);
    }
  }
  return addr;
}

}  // namespace

int main(int argc, char** argv) {
  const std::string listen_addr = ParseListen(argc, argv);

  evrp::device::StubDeviceHost host;
  return evrp::device::api::RunDeviceServer(listen_addr, host);
}
