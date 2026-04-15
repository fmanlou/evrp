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

Playback::Playback(const run_options &options)
    : options_(options), event_writer_(&fs_) {}

int Playback::run() {
  if (options_.playback_path.empty()) {
    logError("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playback_path;
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    g_logger->setLevel(options_.log_level);
    int err = evrp::lua::runScript(path.c_str());
    return (err == LUA_OK) ? 0 : 1;
  }

  if (!fs_.openInput(path)) {
    logError(fs_.errorMessage());
    return 1;
  }

  g_logger->setLevel(options_.log_level);
  g_logger->info("Playing back to input devices (Ctrl+C to stop)...");
  SigintGuard sigint;

  std::istream &input = fs_.inputStream();
  std::string line;
  long long elapsed_us = 0;
  bool is_first_event = true;
  long long leading_delta_us = -1;
  long long trailing_delta_us = -1;

  while (!sigint.stopRequested() && std::getline(input, line)) {
    if (line.empty()) continue;

    std::string label = parseEventLabel(line);
    long long delta_us_val = 0;
    if (label == "leading") {
      if (parseLeadingLine(line, &delta_us_val)) {
        leading_delta_us = delta_us_val;
      }
      continue;
    }
    if (label == "trailing") {
      if (parseTrailingLine(line, &delta_us_val)) {
        trailing_delta_us = delta_us_val;
      }
      continue;
    }

    evrp::device::api::DeviceKind device = evrp::device::api::deviceKindFromLabel(label);
    long long delta_us = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    bool is_event = (device != evrp::device::api::DeviceKind::kUnspecified) &&
                    parseEventLine(line, &delta_us, &type, &code, &value);

    if (!is_event) {
      int err = evrp::lua::executeChunk(&event_writer_, line.c_str());
      if (err != LUA_OK) {
        logError("Lua execution failed, skipping line");
      }
      continue;
    }

    if (is_first_event && options_.execute_wait_before_first &&
        leading_delta_us > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(leading_delta_us));
    }

    long long sleep_us = delta_us - elapsed_us;
    if (sleep_us > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    }
    elapsed_us = delta_us;
    is_first_event = false;

    if (!event_writer_.write(device, type, code, value)) return 1;
  }

  if (options_.execute_wait_after_last && trailing_delta_us > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(trailing_delta_us));
  }

  return 0;
}
