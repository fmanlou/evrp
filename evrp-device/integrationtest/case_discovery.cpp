#include <gtest/gtest.h>

#include "device_integration_fixture.h"
#include "integration_gflags_declare.h"

TEST_F(DeviceIntegration, UdpDiscovery) {
  const auto& e = IntegrationHarness::env();
  if (e.discovery_udp_port <= 0) {
    GTEST_SKIP() << "Requires --discovery_port > 0 matching evrp-device";
  }
  if (!FLAGS_test_udp_discovery) {
    GTEST_SKIP() << "--test_udp_discovery=false";
  }
  const bool manualDiscoveryOnly = !e.spawned_local && e.target.empty();
  if (!IntegrationHarness::runUdpDiscoveryTest()) {
    if (manualDiscoveryOnly) {
      GTEST_SKIP() << "No evrp-device answered UDP discovery (same --discovery_port). "
                      "Start evrp-device on the LAN, use --device_binary, or "
                      "--target=HOST:PORT. Bridge-mode Docker cannot be reached "
                      "via discovery from the host; use --network=host or --target.";
    }
    FAIL() << "UDP discovery failed";
  }
}
