#include <gtest/gtest.h>
#include <vector>

#include "device_integration_fixture.h"
#include "integration_gflags_declare.h"

TEST_F(DeviceIntegration, Playback) {
  if (!IntegrationHarness::hasDirectTarget()) {
    GTEST_SKIP() << "Requires --target / --host+--port or --device_binary";
  }
  auto client = IntegrationHarness::connectDirectClient(FLAGS_rpc_wait_ms);
  ASSERT_TRUE(client);
  ASSERT_TRUE(
      IntegrationHarness::waitUntilGetCapabilitiesOk(*client, FLAGS_rpc_wait_ms));
  std::vector<evrp::device::api::DeviceKind> caps;
  ASSERT_TRUE(IntegrationHarness::fetchCapabilities(*client, &caps));
  ASSERT_TRUE(IntegrationHarness::runPlaybackTest(*client, caps));
}
