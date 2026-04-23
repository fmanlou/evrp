#include <gflags/gflags.h>
#include <string>
#include <vector>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/server/localcursorposition.h"
#include "evrp/device/server/localplayback.h"
#include "evrp/device/server/localinputdevicekindsprovider.h"
#include "evrp/sdk/ioc.h"

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

}  

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp_playback_test_server");
  logService = &logSvc;

  gflags::SetUsageMessage("evrp_playback_test_server — LocalPlayback + gRPC (playback only)");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  NoInputListener noInput;
  evrp::device::server::LocalCursorPosition cursorPosition;
  evrp::device::server::LocalInputDeviceKindsProvider deviceKindsProvider;
  evrp::device::server::LocalPlayback playback;
  evrp::Ioc ioc;
  ioc.emplace<evrp::device::api::IInputListener>(
      static_cast<evrp::device::api::IInputListener*>(&noInput));
  ioc.emplace<evrp::device::api::ICursorPosition>(
      static_cast<evrp::device::api::ICursorPosition*>(&cursorPosition));
  ioc.emplace<evrp::device::api::IInputDeviceKindsProvider>(
      static_cast<evrp::device::api::IInputDeviceKindsProvider*>(
          &deviceKindsProvider));
  ioc.emplace<evrp::device::api::IPlayback>(
      static_cast<evrp::device::api::IPlayback*>(&playback));
  return evrp::device::api::runDeviceServer(FLAGS_listen, ioc);
}
