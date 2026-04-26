#include "playback.h"

#include <chrono>
#include <istream>
#include <string>
#include <thread>
#include <vector>

#include "evrp/device/api/playback.h"
#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "logger.h"
#include "lua/luabindings.h"
#include "playbackeventcollector.h"
#include "scopeguard.h"

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

bool expandLuaThenDevice(evrp::device::api::IPlayback *remote,
                         const char *path_or_chunk, bool from_file) {
  PlaybackEventCollector collector;
  int err =
      from_file ? evrp::lua::playbackLuaFileIntoCollector(path_or_chunk, &collector)
                : evrp::lua::playbackLuaChunkIntoCollector(path_or_chunk, &collector);
  if (err != LUA_OK) {
    return false;
  }
  return collector.uploadAndPlay(remote);
}

}  // namespace

int Playback::run() {
  if (options_.playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playbackPath;
  std::string::size_type dot = path.rfind('.');

  evrp::device::api::IPlayback *remote =
      ioc_.get<evrp::device::api::IPlayback>();
  if (!remote) {
    logError("Ioc has no IPlayback.");
    return 1;
  }

  logService->setLevel(options_.logLevel);

  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    logInfo("Lua → events, playing via evrp-device at {}...", options_.device);
    if (!expandLuaThenDevice(remote, path.c_str(), true)) {
      logError("Lua playback failed.");
      return 1;
    }
    return 0;
  }

  if (!fs_.openInput(path)) {
    logError("{}", fs_.errorMessage());
    return 1;
  }

  logInfo("Recording → events, playing via evrp-device at {} (Ctrl+C tries to stop)...",
          options_.device);

  SigintGuard sigint;
  std::istream &input = fs_.inputStream();
  std::string line;
  long long elapsed_us = 0;
  bool is_first_event = true;
  long long leading_deltaUs = -1;
  long long trailing_deltaUs = -1;
  bool had_content = false;

  while (!sigint.stopRequested() && std::getline(input, line)) {
    if (line.empty()) {
      continue;
    }

    std::string label = parseEventLabel(line);
    long long deltaUs_val = 0;
    if (label == "leading") {
      if (parseLeadingLine(line, &deltaUs_val)) {
        leading_deltaUs = deltaUs_val;
      }
      continue;
    }
    if (label == "trailing") {
      if (parseTrailingLine(line, &deltaUs_val)) {
        trailing_deltaUs = deltaUs_val;
      }
      continue;
    }

    evrp::device::api::DeviceKind device =
        evrp::device::api::deviceKindFromLabel(label);
    long long deltaUs = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    bool is_event = (device != evrp::device::api::DeviceKind::kUnspecified) &&
                    parseEventLine(line, &deltaUs, &type, &code, &value);

    if (!is_event) {
      if (!expandLuaThenDevice(remote, line.c_str(), false)) {
        logError("Lua execution failed, aborting playback");
        return 1;
      }
      had_content = true;
      continue;
    }

    if (is_first_event && options_.executeWaitBeforeFirst &&
        leading_deltaUs > 0) {
      std::this_thread::sleep_for(
          std::chrono::microseconds(leading_deltaUs));
    }

    long long sleep_us = deltaUs - elapsed_us;
    if (sleep_us > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    }
    elapsed_us = deltaUs;
    is_first_event = false;

    evrp::device::api::InputEvent e;
    e.device = device;
    e.timeSec = 0;
    e.timeUsec = 0;
    e.type = static_cast<uint32_t>(type);
    e.code = static_cast<uint32_t>(code);
    e.value = value;
    if (!deviceUploadAndPlay(remote, {e})) {
      return 1;
    }
    had_content = true;
  }

  if (!had_content) {
    logError("No events or Lua in file.");
    return 1;
  }

  if (options_.executeWaitAfterLast && trailing_deltaUs > 0) {
    std::this_thread::sleep_for(
        std::chrono::microseconds(trailing_deltaUs));
  }

  return 0;
}
