#include <memory>
#include <string>

#include "evrp-server/argparser.h"
#include "evrp/device/internal/discovery/devicediscoverysettings.h"
#include "evrp/server/api/server.h"
#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

namespace {

constexpr char kDefaultEvrpServerUnixSocketPath[] = "/tmp/evrp.server.sock";

}  // namespace

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp-server");
  logService = &logSvc;
  auto storage = std::make_shared<MemorySetting>();
  parseServerArgvInto(*storage, argc, argv);
  logService->setLevel(storage->get("logLevel", logging::LogLevel::Info));

  if (FLAGS_listen.empty()) {
    const std::string unixListen =
        std::string("unix:") + kDefaultEvrpServerUnixSocketPath;
    storage->insert(evrp::sdk::kDeviceServerListenAddress, unixListen);
    logInfo("evrp-server: --listen omitted; using default {}", unixListen);
  }
  return evrp::server::makeServer(*storage)->run();
}
