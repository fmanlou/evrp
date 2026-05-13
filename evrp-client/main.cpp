#include "evrp/client/impl/argparser.h"
#include "evrp/client/impl/runner.h"
#include "evrp/sdk/setting/memorysetting.h"
#include "evrp/sdk/logger.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp");
  logService = &logSvc;
  MemorySetting storage;
  parseArgvInto(storage, argc, argv);
  Runner runner(std::move(storage));
  return runner.run();
}
