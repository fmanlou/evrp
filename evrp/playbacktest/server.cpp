#include <gflags/gflags.h>
#include <string>
#include <vector>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/server/localcursorposition.h"
#include "evrp/device/server/localplayback.h"
#include "evrp/device/server/localinputdevicekindsprovider.h"
#include "evrp/ioc.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for PlaybackService and other device RPCs");

namespace {

class NoInputListener final : public evrp::device::api::IInputListener {
 public:
  bool startListening(
      const std::vector<evrp::device::api::DeviceKind>&) override {
    return false;
  }

  std::vector<evrp::device::api::InputEvent> readInputEvents() override {
    return {};
  }

  bool waitForInputEvent(int) override { return false; }

  void cancelListening() override {}

  bool isListening() const override { return false; }
};

}  // namespace

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage("evrp_playback_test_server — LocalPlayback + gRPC (playback only)");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  NoInputListener no_input;
  evrp::device::server::LocalCursorPosition cursor_position;
  evrp::device::server::LocalInputDeviceKindsProvider device_kinds_provider;
  evrp::device::server::LocalPlayback playback;
  evrp::Ioc ioc;
  ioc.emplace(static_cast<evrp::device::api::IInputListener*>(&no_input));
  ioc.emplace(static_cast<evrp::device::api::ICursorPosition*>(&cursor_position));
  ioc.emplace(static_cast<evrp::device::api::IInputDeviceKindsProvider*>(
      &device_kinds_provider));
  ioc.emplace(static_cast<evrp::device::api::IPlayback*>(&playback));
  return evrp::device::api::runDeviceServer(FLAGS_listen, ioc);
}
