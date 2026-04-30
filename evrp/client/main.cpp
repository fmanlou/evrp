#include "evrp/client/runner.h"
#include "evrp/sdk/logger.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp");
  logService = &logSvc;
  Runner runner;
  return runner.run(argc, argv);
}
