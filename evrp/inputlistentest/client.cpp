#include <gflags/gflags.h>

#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "evrp/device/api/client.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/sessionclient.h"
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

bool appendKind(const std::string& token,
                std::vector<evrp::device::api::DeviceKind>* out) {
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
      logError("evrp_inputlisten_test_client: unknown kind \"{}\"", t);
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

void traceInputEvents(int round,
                      const std::vector<evrp::device::api::InputEvent>& batch) {
  for (size_t j = 0; j < batch.size(); ++j) {
    const evrp::device::api::InputEvent& e = batch[j];
    logInfo(
        "  event round={} index={} device={} time={}.{:06d} type={:#x} "
        "code={:#x} value={}",
        round,
        j,
        deviceKindName(e.device),
        e.timeSec,
        e.timeUsec,
        e.type,
        e.code,
        e.value);
  }
}

}  

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp_inputlisten_test_client");
  logService = &logSvc;

  gflags::SetUsageMessage(
      "evrp_inputlisten_test_client — RemoteInputListener against "
      "InputListenService");
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

  const std::shared_ptr<grpc::Channel> channel =
      evrp::sdk::makeGrpcClientChannel(FLAGS_target);
  evrp::sdk::SessionInfo session;
  if (!evrp::sdk::sessionConnect(channel, &session)) {
    logError(
        "evrp_inputlisten_test_client: SessionService/Connect failed");
    return 1;
  }
  const std::unique_ptr<evrp::device::api::IInputListener> listener =
      evrp::device::api::makeRemoteInputListener(channel, session.sessionId);

  if (!listener->startListening(kinds)) {
    logError(
        "evrp_inputlisten_test_client: startListening failed (no devices "
        "or server error?). Is evrp_inputlisten_test_server running on {}?",
        FLAGS_target);
    (void)evrp::sdk::sessionDisconnect(channel, session.sessionId);
    return 1;
  }

  logInfo(
      "evrp_inputlisten_test_client: listening; generate input or "
      "wait for timeouts...");
  int total_events = 0;
  for (int i = 0; i < FLAGS_rounds; ++i) {
    if (listener->waitForInputEvent(FLAGS_wait_ms)) {
      std::vector<evrp::device::api::InputEvent> batch =
          listener->readInputEvents();
      total_events += static_cast<int>(batch.size());
      logInfo("round {} events={}", i, batch.size());
      traceInputEvents(i, batch);
    } else {
      logInfo("round {} no event (timeout or not listening)", i);
    }
  }

  listener->cancelListening();
  logInfo("evrp_inputlisten_test_client: done, total events={}",
          total_events);
  (void)evrp::sdk::sessionDisconnect(channel, session.sessionId);
  return 0;
}
