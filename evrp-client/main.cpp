#include <memory>
#include <string>

#include "evrp-client/argparser.h"
#include "evrp-client/runner.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/server/api/client.h"

namespace {

constexpr char kDefaultEvrpServerUnixSocketPath[] = "/tmp/evrp.server.sock";

}  // namespace

int main(int argc, char* argv[]) {
  logging::LogService logSvc("evrp-client");
  logService = &logSvc;
  auto storage = std::make_shared<MemorySetting>();
  parseArgvInto(*storage, argc, argv);

  if (FLAGS_host.empty()) {
    FLAGS_host = std::string("unix:") + kDefaultEvrpServerUnixSocketPath;
  }

  std::unique_ptr<evrp::server::Client> client =
      evrp::server::makeClient(FLAGS_host);
  if (!client) {
    logError("evrp-client: failed to create client for {}", FLAGS_host);
    return 1;
  }
  evrp::server::Evrp* evrp = client->evrp();
  if (!evrp) {
    logError("evrp-client: Evrp backend is null.");
    return 1;
  }

  Runner runner(storage, evrp);
  return runner.run();
}
