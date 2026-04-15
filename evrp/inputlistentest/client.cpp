// IInputListener 端到端测试：连接远端 InputListenService，走 RemoteInputListener。

#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/device/client/remoteinputlistener.h"
#include "logger.h"

DEFINE_string(target, "127.0.0.1:50051", "Server address (host:port)");
DEFINE_string(
    kinds, "keyboard,mouse,touchpad,touchscreen",
    "Comma-separated device kinds: keyboard,mouse,touchpad,touchscreen "
    "(default: all four)");
DEFINE_int32(wait_ms, 2500,
             "waitForInputEvent timeout per round (ms), must be >= 0");
DEFINE_int32(rounds, 5,
             "Number of wait/read rounds after startListening (>= 1)");

namespace {

bool appendKind(const std::string& token, std::vector<evrp::device::api::DeviceKind>* out) {
  if (token == "keyboard") {
    out->push_back(evrp::device::api::DeviceKind::kKeyboard);
    return true;
  }
  if (token == "mouse") {
    out->push_back(evrp::device::api::DeviceKind::kMouse);
    return true;
  }
  if (token == "touchpad") {
    out->push_back(evrp::device::api::DeviceKind::kTouchpad);
    return true;
  }
  if (token == "touchscreen") {
    out->push_back(evrp::device::api::DeviceKind::kTouchscreen);
    return true;
  }
  return false;
}

bool parseKinds(const std::string& csv,
                 std::vector<evrp::device::api::DeviceKind>* kinds) {
  kinds->clear();
  std::string t;
  std::istringstream in(csv);
  while (std::getline(in, t, ',')) {
    while (!t.empty() && t.front() == ' ') {
      t.erase(0, 1);
    }
    while (!t.empty() && t.back() == ' ') {
      t.pop_back();
    }
    if (t.empty()) {
      continue;
    }
    if (!appendKind(t, kinds)) {
      logError("evrp_inputlisten_test_client: unknown kind \"" + t + "\"");
      return false;
    }
  }
  return true;
}

const char* deviceKindName(evrp::device::api::DeviceKind k) {
  using evrp::device::api::DeviceKind;
  switch (k) {
    case DeviceKind::kTouchpad:
      return "touchpad";
    case DeviceKind::kTouchscreen:
      return "touchscreen";
    case DeviceKind::kMouse:
      return "mouse";
    case DeviceKind::kKeyboard:
      return "keyboard";
    default:
      return "unspecified";
  }
}

void traceInputEvents(int round, const std::vector<evrp::device::api::InputEvent>& batch) {
  for (size_t j = 0; j < batch.size(); ++j) {
    const evrp::device::api::InputEvent& e = batch[j];
    std::ostringstream line;
    line << "  event round=" << round << " index=" << j
         << " device=" << deviceKindName(e.device) << " time=" << e.timeSec << "."
         << std::setfill('0') << std::setw(6) << e.timeUsec << std::setfill(' ')
         << " type=0x" << std::hex << e.type << " code=0x" << e.code << std::dec
         << " value=" << e.value;
    logInfo(line.str());
  }
}

}  // namespace

int main(int argc, char** argv) {
  Logger logger;
  gLogger = &logger;

  gflags::SetUsageMessage(
      "evrp_inputlisten_test_client — RemoteInputListener against InputListenService");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_wait_ms < 0) {
    logError("evrp_inputlisten_test_client: --wait_ms must be >= 0");
    return 2;
  }
  if (FLAGS_rounds < 1) {
    logError("evrp_inputlisten_test_client: --rounds must be >= 1");
    return 2;
  }

  std::vector<evrp::device::api::DeviceKind> kinds;
  if (!parseKinds(FLAGS_kinds, &kinds)) {
    return 2;
  }
  if (kinds.empty()) {
    logError("evrp_inputlisten_test_client: no device kinds after parse");
    return 2;
  }

  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
      FLAGS_target, grpc::InsecureChannelCredentials());
  evrp::device::client::RemoteInputListener listener(channel);

  if (!listener.startListening(kinds)) {
    logError(
        "evrp_inputlisten_test_client: startListening failed (no devices "
        "or server error?). Is evrp_inputlisten_test_server running on " +
        FLAGS_target + "?");
    return 1;
  }

  logInfo(
      "evrp_inputlisten_test_client: listening; generate input or "
      "wait for timeouts...");
  int total_events = 0;
  for (int i = 0; i < FLAGS_rounds; ++i) {
    if (listener.waitForInputEvent(FLAGS_wait_ms)) {
      std::vector<evrp::device::api::InputEvent> batch =
          listener.readInputEvents();
      total_events += static_cast<int>(batch.size());
      logInfo("round " + std::to_string(i) +
               " events=" + std::to_string(batch.size()));
      traceInputEvents(i, batch);
    } else {
      logInfo("round " + std::to_string(i) + " no event (timeout or not listening)");
    }
  }

  listener.cancelListening();
  logInfo("evrp_inputlisten_test_client: done, total events=" +
           std::to_string(total_events));
  return 0;
}
