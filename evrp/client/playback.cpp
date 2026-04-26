#include "playback.h"

#include <string>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/luaeventcomposer/luaeventcomposer.h"
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
  evrp::device::api::OperationResult uploadResult;
  if (!remote->upload(events, &uploadResult) || uploadResult.code != 0) {
    logError("Upload to evrp-device failed (code={}): {}", uploadResult.code,
             uploadResult.message);
    return false;
  }
  evrp::device::api::OperationResult playResult;
  if (!remote->playback(&playResult) || playResult.code != 0) {
    logError("Playback failed (code={}): {}", playResult.code, playResult.message);
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

  int inFd = fs_.openInput(path);
  if (inFd < 0) {
    logError("{}", fs_.errorMessage());
    return 1;
  }
  struct InputFdGuard {
    FileSystem *fs;
    int fd;
    ~InputFdGuard() {
      if (fd >= 0) {
        fs->closeFd(fd);
      }
    }
  } inputFdGuard{&fs_, inFd};

  std::string content;
  if (!fs_.readInputAll(inFd, &content)) {
    logError("Failed to read replay file.");
    return 1;
  }

  LuaEventComposer eventComposer;
  std::vector<evrp::device::api::InputEvent> events;
  const int composeErr = eventComposer.toEvents(content, &events);
  if (composeErr != 0) {
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
