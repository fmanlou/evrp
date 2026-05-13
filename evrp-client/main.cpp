#include <memory>

#include "evrp-client/argparser.h"
#include "evrp-client/runner.h"
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
