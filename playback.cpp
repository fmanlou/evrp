#include "playback.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include "asynclogwriter.h"
#include "eventformat.h"
#include "evdev.h"
#include "filesystem.h"
#include "scopeguard.h"

Playback::Playback(const run_options& options)
    : options_(options), event_writer_(&fs_) {}

int Playback::run() {
  if (options_.playback_path.empty()) {
    std::cerr << "Playback mode requires a file path after -p." << std::endl;
    return 1;
  }
  if (!fs_.open_input(options_.playback_path)) {
    std::cerr << fs_.error_message() << std::endl;
    return 1;
  }

  std::cout << "Playing back to input devices (Ctrl+C to stop)..." << std::endl;
  if (!options_.quiet) log_writer_.start();
  SigintGuard sigint;

  std::istream& input = fs_.input_stream();
  std::string line;
  long long prev_timestamp_us = 0;
  bool has_prev = false;

  while (!sigint.stop_requested() && std::getline(input, line)) {
    if (line.empty()) continue;

    std::string label = parse_event_label(line);
    long long timestamp_us = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    if (!parse_event_line(line, &timestamp_us, &type, &code, &value))
      continue;

    if (label != "keyboard" && label != "mouse" && label != "touchpad") continue;

    if (has_prev && timestamp_us > prev_timestamp_us) {
      std::this_thread::sleep_for(
          std::chrono::microseconds(timestamp_us - prev_timestamp_us));
    }
    prev_timestamp_us = timestamp_us;
    has_prev = true;

    if (label == "keyboard") {
      if (!event_writer_.write_keyboard(type, code, value)) return 1;
    } else if (label == "mouse") {
      if (!event_writer_.write_mouse(type, code, value)) return 1;
    } else {
      if (!event_writer_.write_touchpad(type, code, value)) return 1;
    }
    if (!options_.quiet) log_writer_.push(line);
  }

  log_writer_.stop();
  return 0;
}
