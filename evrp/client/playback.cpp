#include "playback.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "evrp/device/api/client.h"
#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputeventwriter.h"
#include "logger.h"
#include "lua/luabindings.h"
#include "scopeguard.h"

Playback::Playback(const RunOptions &options)
    : options_(options), eventWriter_(&fs_) {}

int Playback::runWithDeviceClient(evrp::device::api::IClient *client) {
  if (options_.playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playbackPath;
  std::string::size_type dot = path.rfind('.');

  if (!client) {
    logError("No device client.");
    return 1;
  }
  evrp::device::api::IPlayback *remote = client->playback();
  if (!remote) {
    logError("No playback handle.");
    return 1;
  }

  logService->setLevel(options_.logLevel);

  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    FileSystem fs;
    InputEventWriter writer(&fs);
    writer.setRemotePlayback(remote);
    logInfo("Lua (local parse) injecting via evrp-device at {}...",
            options_.device);
    int err = evrp::lua::runScriptWithWriter(path.c_str(), &writer);
    return (err == LUA_OK) ? 0 : 1;
  }

  if (!fs_.openInput(path)) {
    logError("{}", fs_.errorMessage());
    return 1;
  }

  logInfo("Playing back via evrp-device at {} (Ctrl+C tries to stop)...",
          options_.device);

  FileSystem fs_lua;
  InputEventWriter lua_writer(&fs_lua);
  lua_writer.setRemotePlayback(remote);

  auto flush_one = [&](const evrp::device::api::InputEvent &e) -> bool {
    const std::vector<evrp::device::api::InputEvent> one{e};
    evrp::device::api::OperationResult up;
    if (!remote->upload(one, &up) || up.code != 0) {
      logError("Upload to evrp-device failed (code={}): {}", up.code,
               up.message);
      return false;
    }
    evrp::device::api::OperationResult play;
    if (!remote->playback(&play) || play.code != 0) {
      logError("Playback failed (code={}): {}", play.code, play.message);
      return false;
    }
    return true;
  };

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
      int err = evrp::lua::executeChunk(&lua_writer, line.c_str());
      if (err != LUA_OK) {
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
    if (!flush_one(e)) {
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

int Playback::run() {
  if (options_.playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playbackPath;
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    logService->setLevel(options_.logLevel);
    int err = evrp::lua::runScript(path.c_str());
    return (err == LUA_OK) ? 0 : 1;
  }

  if (!fs_.openInput(path)) {
    logError("{}", fs_.errorMessage());
    return 1;
  }

  logService->setLevel(options_.logLevel);
  logInfo("Playing back to input devices (Ctrl+C to stop)...");
  SigintGuard sigint;

  std::istream &input = fs_.inputStream();
  std::string line;
  long long elapsed_us = 0;
  bool is_first_event = true;
  long long leading_deltaUs = -1;
  long long trailing_deltaUs = -1;

  while (!sigint.stopRequested() && std::getline(input, line)) {
    if (line.empty()) continue;

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

    evrp::device::api::DeviceKind device = evrp::device::api::deviceKindFromLabel(label);
    long long deltaUs = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    bool is_event = (device != evrp::device::api::DeviceKind::kUnspecified) &&
                    parseEventLine(line, &deltaUs, &type, &code, &value);

    if (!is_event) {
      int err = evrp::lua::executeChunk(&eventWriter_, line.c_str());
      if (err != LUA_OK) {
        logError("Lua execution failed, skipping line");
      }
      continue;
    }

    if (is_first_event && options_.executeWaitBeforeFirst &&
        leading_deltaUs > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(leading_deltaUs));
    }

    long long sleep_us = deltaUs - elapsed_us;
    if (sleep_us > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    }
    elapsed_us = deltaUs;
    is_first_event = false;

    if (!eventWriter_.write(device, type, code, value)) return 1;
  }

  if (options_.executeWaitAfterLast && trailing_deltaUs > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(trailing_deltaUs));
  }

  return 0;
}
