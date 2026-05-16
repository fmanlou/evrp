#include <memory>

#include "evrp-client/argparser.h"
#include "evrp-client/runner.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/sessionclient.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/server/impl/client/remoteevrp.h"

int main(int argc, char* argv[]) {
  logging::LogService logSvc("evrp-client");
  logService = &logSvc;
  auto storage = std::make_shared<MemorySetting>();
  parseArgvInto(*storage, argc, argv);

  if (FLAGS_host.empty()) {
    logService->setLevel(storage->get("logLevel", logging::LogLevel::Info));
    logError(
        "evrp-client: --host=HOST:PORT is required (address of evrp-server "
        "EvrpService).");
    printUsage(storage->get<std::string>("program", "evrp-client").c_str());
    return 1;
  }

  std::shared_ptr<grpc::Channel> channel =
      evrp::sdk::makeGrpcClientChannel(FLAGS_host);
  if (!channel) {
    logError("evrp-client: failed to create gRPC channel for {}", FLAGS_host);
    return 1;
  }

  evrp::server::RemoteEvrp evrp(std::move(channel));
  Runner runner(storage, &evrp);
  return runner.run();
}
