#include <gtest/gtest.h>

#include "device_integration_fixture.h"
#include "integration_gflags_declare.h"

TEST_F(DeviceIntegration, UdpDiscovery) {
  const auto& e = IntegrationHarness::env();
  if (e.discovery_udp_port <= 0) {
    GTEST_SKIP() << "Requires local evrp-device spawn (--device_binary)";
  }
  if (!FLAGS_test_udp_discovery) {
    GTEST_SKIP() << "--test_udp_discovery=false";
  }
  ASSERT_TRUE(IntegrationHarness::runUdpDiscoveryTest());
}
