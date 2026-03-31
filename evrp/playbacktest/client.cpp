#include <gflags/gflags.h>
#include <grpcpp/grpcpp.h>
#include <linux/input-event-codes.h>

#include <string>
#include <thread>
#include <vector>

#include "evrp/countingsemaphore.h"
#include "evrp/device/api/types.h"
#include "evrp/device/client/remoteplayback.h"
#include "logger.h"

DEFINE_string(target, "127.0.0.1:50051", "Server address (host:port)");
DEFINE_bool(progress, true,
            "Use SubscribePlayback progress (CountingSemaphore) alongside "
            "Unary Playback");

namespace {

unsigned short key_code_for_char(char c) {
  switch (c) {
    case 'h':
      return KEY_H;
    case 'e':
      return KEY_E;
    case 'l':
      return KEY_L;
    case 'o':
      return KEY_O;
    case ' ':
      return KEY_SPACE;
    case 'w':
      return KEY_W;
    case 'r':
      return KEY_R;
    case 'd':
      return KEY_D;
    default:
      return 0;
  }
}

std::vector<evrp::device::api::InputEvent> make_hello_world_key_events() {
  const char kPhrase[] = "hello world";
  std::vector<evrp::device::api::InputEvent> v;
  int usec = 0;
  for (size_t i = 0; kPhrase[i] != '\0'; ++i) {
    const unsigned short key = key_code_for_char(kPhrase[i]);
    if (key == 0) {
      continue;
    }
    for (int value : {1, 0}) {
      evrp::device::api::InputEvent e;
      e.device = evrp::device::api::DeviceKind::kKeyboard;
      e.time_sec = 0;
      e.time_usec = usec;
      e.type = static_cast<uint32_t>(EV_KEY);
      e.code = static_cast<uint32_t>(key);
      e.value = value;
      v.push_back(e);
      usec += 12000;
    }
  }
  return v;
}

}  // namespace

int main(int argc, char** argv) {
  Logger logger;
  g_logger = &logger;

  gflags::SetUsageMessage(
      "evrp_playback_test_client — replay keyboard events for \"hello world\"");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const auto events = make_hello_world_key_events();
  log_info("evrp_playback_test_client: will replay " + std::to_string(events.size()) +
           " key events (EV_KEY press/release) for \"hello world\"");

  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(FLAGS_target, grpc::InsecureChannelCredentials());
  evrp::device::client::RemotePlayback remote(channel);

  evrp::device::api::OperationResult up;
  if (!remote.upload(events, &up)) {
    log_error("evrp_playback_test_client: upload failed code=" +
              std::to_string(up.code) + " msg=" + up.message);
    return 1;
  }

  evrp::CountingSemaphore progress_sem;
  std::thread consumer;
  const int n = static_cast<int>(events.size());

  if (FLAGS_progress) {
    consumer = std::thread([&]() {
      for (int i = 0; i < n; ++i) {
        progress_sem.acquire();
        log_info("evrp_playback_test_client: progress idx=" +
                 std::to_string(remote.playback_index()));
      }
    });
  }

  evrp::device::api::OperationResult play;
  const bool ok =
      remote.playback(&play, FLAGS_progress ? &progress_sem : nullptr);

  if (consumer.joinable()) {
    consumer.join();
  }

  if (!ok) {
    log_error("evrp_playback_test_client: playback failed code=" +
              std::to_string(play.code) + " msg=" + play.message);
    return 1;
  }
  log_info("hello world");
  log_info(
      "evrp_playback_test_client: playback finished (keyboard injection "
      "requires focus in a text field)");
  return 0;
}
