// IInputListener 端到端测试：连接远端 InputListenService，走 RemoteInputListener。

#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/device/client/remoteinputlistener.h"

DEFINE_string(target, "127.0.0.1:50051", "Server address (host:port)");
DEFINE_string(
    kinds, "keyboard,mouse,touchpad,touchscreen",
    "Comma-separated device kinds: keyboard,mouse,touchpad,touchscreen "
    "(default: all four)");
DEFINE_int32(wait_ms, 2500,
             "wait_for_input_event timeout per round (ms), must be >= 0");
DEFINE_int32(rounds, 5,
             "Number of wait/read rounds after start_listening (>= 1)");

namespace {

bool append_kind(const std::string& token, std::vector<evrp::device::api::DeviceKind>* out) {
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

std::vector<evrp::device::api::DeviceKind> parse_kinds(const std::string& csv) {
  std::vector<evrp::device::api::DeviceKind> kinds;
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
    if (!append_kind(t, &kinds)) {
      std::cerr << "evrp_inputlisten_test_client: unknown kind \"" << t << "\"\n";
      std::exit(2);
    }
  }
  return kinds;
}

const char* device_kind_name(evrp::device::api::DeviceKind k) {
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

void log_input_events(int round, const std::vector<evrp::device::api::InputEvent>& batch) {
  for (size_t j = 0; j < batch.size(); ++j) {
    const evrp::device::api::InputEvent& e = batch[j];
    std::cout << "  event round=" << round << " index=" << j
              << " device=" << device_kind_name(e.device) << " time=" << e.time_sec << "."
              << std::setfill('0') << std::setw(6) << e.time_usec << std::setfill(' ')
              << " type=0x" << std::hex << e.type << " code=0x" << e.code << std::dec
              << " value=" << e.value << '\n';
  }
}

}  // namespace

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
      "evrp_inputlisten_test_client — RemoteInputListener against InputListenService");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_wait_ms < 0) {
    std::cerr << "evrp_inputlisten_test_client: --wait_ms must be >= 0\n";
    return 2;
  }
  if (FLAGS_rounds < 1) {
    std::cerr << "evrp_inputlisten_test_client: --rounds must be >= 1\n";
    return 2;
  }

  std::vector<evrp::device::api::DeviceKind> kinds = parse_kinds(FLAGS_kinds);
  if (kinds.empty()) {
    std::cerr << "evrp_inputlisten_test_client: no device kinds after parse\n";
    return 2;
  }

  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
      FLAGS_target, grpc::InsecureChannelCredentials());
  evrp::device::client::RemoteInputListener listener(channel);

  if (!listener.start_listening(kinds)) {
    std::cerr
        << "evrp_inputlisten_test_client: start_listening failed (no devices "
           "or server error?). Is evrp_inputlisten_test_server running on "
        << FLAGS_target << "?\n";
    return 1;
  }

  std::cout << "evrp_inputlisten_test_client: listening; generate input or "
               "wait for timeouts...\n";
  int total_events = 0;
  for (int i = 0; i < FLAGS_rounds; ++i) {
    if (listener.wait_for_input_event(FLAGS_wait_ms)) {
      std::vector<evrp::device::api::InputEvent> batch =
          listener.read_input_events();
      total_events += static_cast<int>(batch.size());
      std::cout << "round " << i << " events=" << batch.size() << '\n';
      log_input_events(i, batch);
    } else {
      std::cout << "round " << i << " no event (timeout or not listening)\n";
    }
  }

  listener.cancel_listening();
  std::cout << "evrp_inputlisten_test_client: done, total events=" << total_events
            << "\n";
  return 0;
}
