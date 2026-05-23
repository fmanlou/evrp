#include "evrp/server/impl/server/playback.h"

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/sdk/types.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/luaeventcomposer/luaeventcomposer.h"
#include "evrp/sdk/evdev.h"
#include "evrp/sdk/log/logger.h"
#include "evrp/sdk/scopeguard.h"
#include "evrp/sdk/setting/isetting.h"

Playback::Playback(std::shared_ptr<ISetting> setting,
                   evrp::device::api::IPlayback *playback,
                   IEnhancedFileSystem *fs)
    : remote_(playback), fs_(fs) {
  if (!setting) {
    return;
  }
  logLevel_ = setting->get("logLevel", logging::LogLevel::Info);
  playbackPath_ = setting->get<std::string>("playbackPath", {});
  device_ = setting->get<std::string>("device", {});
}

Playback::Playback(std::shared_ptr<ISetting> setting, const evrp::Ioc &ioc)
    : Playback(std::move(setting), ioc.get<evrp::device::api::IPlayback>(),
               ioc.get<IEnhancedFileSystem>()) {}

namespace {

bool deviceUploadAndPlay(evrp::device::api::IPlayback *remote,
                         const std::vector<evrp::sdk::InputEvent> &events) {
  if (events.empty()) {
    return true;
  }
  evrp::sdk::StatusCode uploadResult;
  if (!remote->upload(events, &uploadResult) || uploadResult.code != 0) {
    logError("Upload to evrp-device failed (code={}): {}", uploadResult.code,
             uploadResult.message);
    return false;
  }
  evrp::sdk::StatusCode playResult;
  if (!remote->playback(&playResult) || playResult.code != 0) {
    logError("Playback failed (code={}): {}", playResult.code, playResult.message);
    return false;
  }
  return true;
}

}  // namespace

int Playback::run() {
  if (playbackPath_.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  if (!remote_) {
    logError("Playback has no IPlayback.");
    return 1;
  }
  if (!fs_) {
    logError("Playback has no IEnhancedFileSystem.");
    return 1;
  }

  logService->setLevel(logLevel_);

  int inFd = fs_->openFd(playbackPath_, O_RDONLY, 0);
  if (inFd < 0) {
    int err = errno;
    logError("Failed to open input file {}: {}", playbackPath_, strerror(err));
    return 1;
  }
  evrp::sdk::ScopeGuard closeInputFd{[fs = fs_, fd = inFd]() {
    if (fd >= 0) {
      fs->closeFd(fd);
    }
  }};

  std::string content;
  if (!fs_->readInputAll(inFd, &content)) {
    logError("Failed to read replay file.");
    return 1;
  }

  auto eventComposer = std::make_unique<LuaEventComposer>();
  std::vector<evrp::sdk::InputEvent> events;
  const int composeErr = eventComposer->toEvents(content, &events);
  if (composeErr != 0) {
    logError("Replay compose failed (e.g. Lua error).");
    return 1;
  }

  if (events.empty()) {
    logError("No events or Lua in file.");
    return 1;
  }

  logInfo("Replay text → events, playing via evrp-device at {} (Ctrl+C tries to stop)...",
          device_);

  SigintGuard sigint;
  if (sigint.stopRequested()) {
    return 1;
  }

  if (!deviceUploadAndPlay(remote_, events)) {
    return 1;
  }

  return 0;
}
