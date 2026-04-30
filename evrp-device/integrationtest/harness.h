#pragma once

#include <memory>
#include <string>
#include <vector>

#include "evrp/device/api/client.h"
#include "evrp/device/api/types.h"

struct IntegrationEnv {
  std::string target;
  int discovery_udp_port{-1};
  bool spawned_local{false};
};

class IntegrationHarness {
 public:
  static bool initialize();
  static void shutdown();
  static const IntegrationEnv& env();

  static std::unique_ptr<evrp::device::api::IClient> connectDirectClient(
      int timeout_ms);

  static bool waitUntilGetCapabilitiesOk(evrp::device::api::IClient& client,
                                        int total_timeout_ms);
  static bool fetchCapabilities(
      evrp::device::api::IClient& client,
      std::vector<evrp::device::api::DeviceKind>* kinds_out);

  static bool runInputListenTest(
      evrp::device::api::IClient& client,
      const std::vector<evrp::device::api::DeviceKind>& caps);
  static bool runPlaybackTest(
      evrp::device::api::IClient& client,
      const std::vector<evrp::device::api::DeviceKind>& caps);
  static bool runUdpDiscoveryTest();

 private:
  static IntegrationEnv env_;
  static bool initialized_;
};
