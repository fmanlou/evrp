#include <gflags/gflags.h>
#include <linux/input-event-codes.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "evrp/sdk/countingsemaphore.h"
#include "evrp/device/api/client.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/sessionclient.h"
#include "logger.h"

DEFINE_string(target, "127.0.0.1:50051", "Server address (host:port)");
DEFINE_bool(progress, true,
            "Use SubscribePlayback progress (CountingSemaphore) alongside "
            "Unary Playback");

namespace {

unsigned short keyCodeForChar(char c) {
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

std::vector<evrp::device::api::InputEvent> makeHelloWorldKeyEvents() {
  const char kPhrase[] = "hello world";
  std::vector<evrp::device::api::InputEvent> v;
  int usec = 0;
  for (size_t i = 0; kPhrase[i] != '\0'; ++i) {
    const unsigned short key = keyCodeForChar(kPhrase[i]);
    if (key == 0) {
      continue;
    }
    for (int value : {1, 0}) {
      evrp::device::api::InputEvent e;
      e.device = evrp::device::api::DeviceKind::kKeyboard;
      e.timeSec = 0;
      e.timeUsec = usec;
      e.type = static_cast<uint32_t>(EV_KEY);
      e.code = static_cast<uint32_t>(key);
      e.value = value;
      v.push_back(e);
      usec += 12000;
    }
  }
  return v;
}

}  

int main(int argc, char** argv) {
  logging::LogService logSvc("evrp_playback_test_client");
  logService = &logSvc;

  gflags::SetUsageMessage(
      "evrp_playback_test_client — replay keyboard events for \"hello world\"");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const auto events = makeHelloWorldKeyEvents();
  logInfo(
      "evrp_playback_test_client: will replay {} key events (EV_KEY "
      "press/release) for \"hello world\"",
      events.size());

  const std::shared_ptr<grpc::Channel> channel =
      evrp::sdk::makeGrpcClientChannel(FLAGS_target);
  evrp::sdk::SessionInfo session;
  if (!evrp::sdk::sessionConnect(channel, &session)) {
    logError("evrp_playback_test_client: SessionService/Connect failed");
    return 1;
  }
  const std::unique_ptr<evrp::device::api::IPlayback> remote =
      evrp::device::api::makeRemotePlayback(channel, session.sessionId);

  evrp::device::api::OperationResult up;
  if (!remote->upload(events, &up)) {
    logError("evrp_playback_test_client: upload failed code={} msg={}",
             up.code,
             up.message);
    (void)evrp::sdk::sessionDisconnect(channel, session.sessionId);
    return 1;
  }

  evrp::CountingSemaphore progress_sem;
  std::thread consumer;
  const int n = static_cast<int>(events.size());

  if (FLAGS_progress) {
    consumer = std::thread([&]() {
      for (int i = 0; i < n; ++i) {
        progress_sem.acquire();
        logInfo("evrp_playback_test_client: progress idx={}",
                remote->playbackIndex());
      }
    });
  }

  evrp::device::api::OperationResult play;
  const bool ok =
      remote->playback(&play, FLAGS_progress ? &progress_sem : nullptr);

  if (consumer.joinable()) {
    consumer.join();
  }

  if (!ok) {
    logError("evrp_playback_test_client: playback failed code={} msg={}",
             play.code,
             play.message);
    (void)evrp::sdk::sessionDisconnect(channel, session.sessionId);
    return 1;
  }
  logInfo("hello world");
  logInfo(
      "evrp_playback_test_client: playback finished (keyboard injection "
      "requires focus in a text field)");
  (void)evrp::sdk::sessionDisconnect(channel, session.sessionId);
  return 0;
}
