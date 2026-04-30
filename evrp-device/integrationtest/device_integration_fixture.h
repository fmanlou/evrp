#pragma once

#include <gtest/gtest.h>

#include "harness.h"

class DeviceIntegration : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    ASSERT_TRUE(IntegrationHarness::initialize())
        << "Set --device_binary or EVRP_DEVICE_BINARY, or --target / --host & "
           "--port";
  }

  static void TearDownTestSuite() { IntegrationHarness::shutdown(); }
};
