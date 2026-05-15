#include <memory>

#include "evrp-server/argparser.h"
#include "evrp/server/api/server.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp-server");
  logService = &logSvc;
  auto storage = std::make_shared<MemorySetting>();
  parseArgvInto(*storage, argc, argv);
  logService->setLevel(storage->get("logLevel", logging::LogLevel::Info));

  const bool recording = storage->get<bool>("recording", false);
  const bool playback = storage->get<bool>("playback", false);
  if (FLAGS_listen.empty()) {
    logError("evrp-server: --listen=HOST:PORT is required.");
    printUsage(storage->get<std::string>("program", "evrp-server").c_str());
    return 1;
  }
  if (recording || playback) {
    logError(
        "evrp-server runs only the EvrpService RPC host; use evrp-client for "
        "--record / --playback.");
    printUsage(storage->get<std::string>("program", "evrp-server").c_str());
    return 1;
  }
  return evrp::server::makeServer(*storage)->run();
}
