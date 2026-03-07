#include "playback.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "asynclogwriter.h"
#include "eventformat.h"
#include "evdev/evdev.h"
#include "filesystem/filesystem.h"
#include "inputdevice.h"
#include "scopeguard.h"

Playback::Playback(const run_options& options) : options_(options) {}

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
  evdev::signal_install_sigint();

  auto cleanup = make_scope_guard([this]() {
    for (const auto& p : label_to_fd_) {
      if (p.second >= 0) fs_.close_fd(p.second);
    }
    evdev::signal_restore_sigint();
  });

  std::istream& input = fs_.input_stream();
  std::string line;
  long long prev_timestamp_us = 0;
  bool has_prev = false;

  while (!evdev::signal_stop_requested() && std::getline(input, line)) {
    if (line.empty()) continue;

    std::string label = parse_event_label(line);
    long long timestamp_us = 0;
    unsigned short type = 0, code = 0;
    int value = 0;
    if (!parse_event_line(line, &timestamp_us, &type, &code, &value))
      continue;

    int fd = get_fd(label);
    if (fd < 0) continue;

    if (has_prev && timestamp_us > prev_timestamp_us) {
      std::this_thread::sleep_for(
          std::chrono::microseconds(timestamp_us - prev_timestamp_us));
    }
    prev_timestamp_us = timestamp_us;
    has_prev = true;

    if (!write_event_with_sync(fd, type, code, value)) return 1;
    if (!options_.quiet) log_writer_.push(line);
  }

  log_writer_.stop();
  return 0;
}

int Playback::get_fd(const std::string& label) {
  if (label.empty()) return -1;
  auto it = label_to_fd_.find(label);
  if (it != label_to_fd_.end()) return it->second;

  std::string dev_path = find_device_path(label);
  if (dev_path.empty()) {
    std::cerr << "No " << label << " device found, skipping events."
              << std::endl;
    label_to_fd_[label] = -1;
    return -1;
  }

  int fd = fs_.open_read_write(dev_path.c_str());
  if (fd < 0) {
    std::cerr << "Failed to open " << dev_path << " for write (try: sudo): ";
    std::perror(dev_path.c_str());
    label_to_fd_[label] = -1;
    return -1;
  }

  label_to_fd_[label] = fd;
  std::cout << "Playing back " << label << " to " << dev_path << std::endl;
  return fd;
}

bool Playback::write_event_with_sync(int fd, unsigned short type,
                                    unsigned short code, int value) {
  if (!write_event(fd, type, code, value)) {
    std::perror("Failed to write event");
    return false;
  }
  if (type != EV_SYN) {
    bool needs_mt =
        (type == EV_ABS &&
         (code == ABS_MT_POSITION_Y ||
          (code == ABS_MT_TRACKING_ID && value == -1)));
    if (needs_mt && !write_event(fd, EV_SYN, SYN_MT_REPORT, 0)) {
      std::perror("Failed to write SYN_MT_REPORT");
      return false;
    }
    if (!write_event(fd, EV_SYN, SYN_REPORT, 0)) {
      std::perror("Failed to write SYN_REPORT");
      return false;
    }
  }
  return true;
}

bool Playback::write_event(int fd, unsigned short type, unsigned short code,
                          int value) {
  struct input_event ev = {};
  gettimeofday(&ev.time, nullptr);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  long n = fs_.write_fd(fd, &ev, sizeof(ev));
  return n == static_cast<long>(sizeof(ev));
}
