#include "playback.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#include "deviceid.h"
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
    log_error("Playback mode requires a file path after -p.");
    return 1;
  }

  const std::string &path = options_.playback_path;
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos && path.substr(dot) == ".lua") {
    g_logger->set_level(options_.log_level);
    int err = evrp::lua::run_script(path.c_str());
    return (err == LUA_OK) ? 0 : 1;
  }

  if (!fs_.open_input(path)) {
    log_error(fs_.error_message());
    return 1;
  }

  g_logger->set_level(options_.log_level);
  g_logger->info("Playing back to input devices (Ctrl+C to stop)...");
  SigintGuard sigint;

  std::istream &input = fs_.input_stream();
  std::string line;
  long long elapsed_us = 0;
  bool is_first_event = true;
  long long leading_delta_us = -1;
  long long trailing_delta_us = -1;

  while (!sigint.stop_requested() && std::getline(input, line)) {
    if (line.empty()) continue;

    std::string label = parse_event_label(line);
    long long delta_us_val = 0;
    if (label == "leading") {
      if (parse_leading_line(line, &delta_us_val)) {
        leading_delta_us = delta_us_val;
      }
      continue;
    }
    if (label == "trailing") {
      if (parse_trailing_line(line, &delta_us_val)) {
        trailing_delta_us = delta_us_val;
      }
      continue;
    }

    DeviceId device_id = device_id_from_label(label);
    long long delta_us = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    bool is_event = (device_id != DeviceId::Unknown) &&
                    parse_event_line(line, &delta_us, &type, &code, &value);

    if (!is_event) {
      int err = evrp::lua::execute_chunk(&event_writer_, line.c_str());
      if (err != LUA_OK) {
        log_error("Lua execution failed, skipping line");
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

    if (!event_writer_.write(device_id, type, code, value)) return 1;
  }

  if (options_.execute_wait_after_last && trailing_delta_us > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(trailing_delta_us));
  }

  return 0;
}
