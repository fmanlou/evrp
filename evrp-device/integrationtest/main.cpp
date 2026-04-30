#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include "evrp/sdk/logger.h"

#include "harness.h"
#include "integration_gflags_declare.h"

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp_device_integration_test");
  logService = &logSvc;

  gflags::SetUsageMessage(
      "Host-side integration tests: connect, GetCapabilities, InputListen, "
      "Playback, UDP discovery (--target or --host/--port; optional "
      "--device_binary for CI)");
  ::testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  logService->setLevel(logLevelFromString(FLAGS_log_level));
  return RUN_ALL_TESTS();
}
