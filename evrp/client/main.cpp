#include "evrp/client/argparser.h"
#include "evrp/client/runner.h"
#include "evrp/sdk/logger.h"

int main(int argc, char *argv[]) {
  logging::LogService logSvc("evrp");
  logService = &logSvc;
  ParsedOptions options;
  parseArgvInto(options, argc, argv);
  Runner runner(std::move(options));
  return runner.run();
}
