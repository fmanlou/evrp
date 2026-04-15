#include "playback.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "logger.h"
#include "lua/luabindings.h"
#include "scopeguard.h"

Playback::Playback(const RunOptions &options)
    : options_(options), eventWriter_(&fs_) {}

int Playback::run() {
  if (options_.playbackPath.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playbackPath;
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    gLogger->setLevel(options_.logLevel);
    int err = evrp::lua::runScript(path.c_str());
    return (err == LUA_OK) ? 0 : 1;
  }

  if (!fs_.openInput(path)) {
    logError(fs_.errorMessage());
    return 1;
  }

  gLogger->setLevel(options_.logLevel);
  gLogger->info("Playing back to input devices (Ctrl+C to stop)...");
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
