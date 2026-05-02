#pragma once

#include <gtest/gtest.h>

#include "harness.h"

class DeviceIntegration : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    ASSERT_TRUE(IntegrationHarness::initialize())
        << "Set --discovery_port (no --target) for discovery-only, or "
           "--target/--host & --port, or --device_binary / EVRP_DEVICE_BINARY";
  }

  static void TearDownTestSuite() { IntegrationHarness::shutdown(); }
};
