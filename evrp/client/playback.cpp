#include "playback.h"

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/luaeventcomposer/luaeventcomposer.h"
#include "evrp/sdk/evdev.h"
#include "evrp/sdk/logger.h"

Playback::Playback(const std::map<std::string, std::any>& parsed,
                   evrp::device::api::IPlayback *playback,
                   IEnhancedFileSystem *fs)
    : parsed_(parsed), remote_(playback), fs_(fs) {}

Playback::Playback(const std::map<std::string, std::any>& parsed,
                   const evrp::Ioc &ioc)
    : Playback(parsed, ioc.get<evrp::device::api::IPlayback>(),
               ioc.get<IEnhancedFileSystem>()) {}

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
  const std::string playbackPath =
      parsed_options::stringOr(parsed_, "playbackPath");
  if (playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = playbackPath;

  if (!remote_) {
    logError("Playback has no IPlayback.");
    return 1;
  }
  if (!fs_) {
    logError("Playback has no IEnhancedFileSystem.");
    return 1;
  }

  logService->setLevel(parsed_options::logLevelOr(parsed_, "logLevel"));

  int inFd = fs_->openFd(path, O_RDONLY, 0);
  if (inFd < 0) {
    int err = errno;
    logError("Failed to open input file {}: {}", path, strerror(err));
    return 1;
  }
  struct InputFdGuard {
    IEnhancedFileSystem *fs;
    int fd;
    ~InputFdGuard() {
      if (fd >= 0) {
        fs->closeFd(fd);
      }
    }
  } inputFdGuard{fs_, inFd};

  std::string content;
  if (!fs_->readInputAll(inFd, &content)) {
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
          parsed_options::stringOr(parsed_, "device"));

  SigintGuard sigint;
  if (sigint.stopRequested()) {
    return 1;
  }

  if (!deviceUploadAndPlay(remote_, events)) {
    return 1;
  }

  return 0;
}
