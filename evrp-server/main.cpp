#include <memory>

#include "evrp-server/argparser.h"
#include "evrp-server/runner.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/setting/memorysetting.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp");
  logService = &logSvc;
  auto storage = std::make_shared<MemorySetting>();
  parseArgvInto(*storage, argc, argv);
  Runner runner(storage);
  return runner.run();
}
