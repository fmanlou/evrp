#include "playback.h"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/eventcomposer.h"
#include "evrp/sdk/evdev.h"
#include "evrp/sdk/logger.h"

Playback::Playback(const RunOptions &options, const evrp::Ioc &ioc)
    : options_(options), ioc_(ioc) {}

namespace {

bool deviceUploadAndPlay(evrp::device::api::IPlayback *remote,
                         const std::vector<evrp::device::api::InputEvent> &events) {
  if (events.empty()) {
    return true;
  }
  evrp::device::api::OperationResult up;
  if (!remote->upload(events, &up) || up.code != 0) {
    logError("Upload to evrp-device failed (code={}): {}", up.code, up.message);
    return false;
  }
  evrp::device::api::OperationResult play;
  if (!remote->playback(&play) || play.code != 0) {
    logError("Playback failed (code={}): {}", play.code, play.message);
    return false;
  }
  return true;
}

}  // namespace

int Playback::run() {
  if (options_.playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playbackPath;

  evrp::device::api::IPlayback *remote =
      ioc_.get<evrp::device::api::IPlayback>();
  if (!remote) {
    logError("Ioc has no IPlayback.");
    return 1;
  }

  logService->setLevel(options_.logLevel);

  if (!fs_.openInput(path)) {
    logError("{}", fs_.errorMessage());
    return 1;
  }

  std::ostringstream buf;
  buf << fs_.inputStream().rdbuf();
  const std::string content = buf.str();

  EventComposer event_composer;
  std::vector<evrp::device::api::InputEvent> events;
  const int cerr = event_composer.toEvents(content, &events);
  if (cerr != 0) {
    logError("Replay compose failed (e.g. Lua error).");
    return 1;
  }

  if (events.empty()) {
    logError("No events or Lua in file.");
    return 1;
  }

  logInfo("Replay text → events, playing via evrp-device at {} (Ctrl+C tries to stop)...",
          options_.device);

  SigintGuard sigint;
  if (sigint.stopRequested()) {
    return 1;
  }

  if (!deviceUploadAndPlay(remote, events)) {
    return 1;
  }

  return 0;
}
