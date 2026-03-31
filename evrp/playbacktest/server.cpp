#include <gflags/gflags.h>
#include <string>
#include <vector>

#include "logger.h"
#include "evrp/device/api/server.h"
#include "evrp/device/api/inputlistener.h"
#include "evrp/device/server/localplayback.h"

DEFINE_string(listen, "127.0.0.1:50051",
              "Listen address (host:port) for PlaybackService and other device RPCs");

namespace {

class NoInputListener final : public evrp::device::api::IInputListener {
 public:
  bool start_listening(
      const std::vector<evrp::device::api::DeviceKind>&) override {
    return false;
  }

  std::vector<evrp::device::api::InputEvent> read_input_events() override {
    return {};
  }

  bool wait_for_input_event(int) override { return false; }

  void cancel_listening() override {}

  bool is_listening() const override { return false; }
};

}  // namespace

int main(int argc, char** argv) {
  Logger logger;
  g_logger = &logger;

  gflags::SetUsageMessage("evrp_playback_test_server — LocalPlayback + gRPC (playback only)");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  NoInputListener no_input;
  evrp::device::server::LocalPlayback playback;
  return evrp::device::api::run_device_server(FLAGS_listen, no_input, playback);
}
