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
#include "evdev/evdev.h"
#include "filesystem/filesystem.h"
#include "inputdevice.h"

namespace {

static std::string parse_label(const std::string& line) {
  std::size_t lb = line.find('[');
  if (lb == std::string::npos) return "";
  std::size_t rb = line.find(']', lb + 1);
  if (rb == std::string::npos) return "";
  return line.substr(lb + 1, rb - lb - 1);
}

static bool parse_event_line(const std::string& line,
                             long long* out_timestamp_us,
                             unsigned short* out_type, unsigned short* out_code,
                             int* out_value) {
  if (!out_type || !out_code || !out_value) return false;

  std::size_t bracket = line.find("] ");
  if (bracket == std::string::npos) return false;
  std::size_t ts_start = bracket + 2;
  std::size_t ts_end = line.find(' ', ts_start);
  if (ts_end == std::string::npos || ts_end <= ts_start) return false;

  std::string ts_token = line.substr(ts_start, ts_end - ts_start);
  std::size_t dot = ts_token.find('.');
  if (dot == std::string::npos || dot == 0 || dot + 1 >= ts_token.size()) {
    return false;
  }
  long long sec = 0, usec = 0;
  try {
    sec = std::stoll(ts_token.substr(0, dot));
    usec = std::stoll(ts_token.substr(dot + 1));
  } catch (...) {
    return false;
  }
  if (out_timestamp_us) *out_timestamp_us = sec * 1000000LL + usec;

  std::size_t type_pos = line.find("type=", ts_end);
  if (type_pos == std::string::npos) return false;
  type_pos += 5;
  std::size_t type_end = line.find_first_of("( ", type_pos);
  if (type_end == std::string::npos) type_end = line.size();
  try {
    *out_type = static_cast<unsigned short>(
        std::stoul(line.substr(type_pos, type_end - type_pos)));
  } catch (...) {
    return false;
  }

  std::size_t code_pos = line.find("code=", type_end);
  if (code_pos == std::string::npos) return false;
  code_pos += 5;
  std::size_t code_end = line.find_first_of("( ", code_pos);
  if (code_end == std::string::npos) code_end = line.size();
  try {
    *out_code = static_cast<unsigned short>(
        std::stoul(line.substr(code_pos, code_end - code_pos)));
  } catch (...) {
    return false;
  }

  std::size_t value_pos = line.find("value=", code_end);
  if (value_pos == std::string::npos) return false;
  value_pos += 6;
  std::size_t value_end = line.find_first_of(" /", value_pos);
  if (value_end == std::string::npos) value_end = line.size();
  try {
    *out_value = std::stoi(line.substr(value_pos, value_end - value_pos));
  } catch (...) {
    return false;
  }
  return true;
}

static std::string find_device_path(const std::string& label) {
  if (label == "touchpad") return find_first_touchpad();
  if (label == "mouse") return find_first_mouse();
  if (label == "keyboard") return find_first_keyboard();
  return "";
}

static bool write_event(const FileSystem* fs, int fd, unsigned short type,
                        unsigned short code, int value) {
  struct input_event ev = {};
  gettimeofday(&ev.time, nullptr);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  long n = fs->write_fd(fd, &ev, sizeof(ev));
  return n == static_cast<long>(sizeof(ev));
}

}  // namespace

int playback_file_to_uinput(const std::string& path, bool quiet) {
  FileSystem fs;
  if (!fs.open_input(path)) {
    std::cerr << fs.error_message() << std::endl;
    return 1;
  }

  std::istream& input = fs.input_stream();
  std::map<std::string, int> label_to_fd;
  std::map<std::string, std::string> label_to_path;

  auto get_fd = [&](const std::string& label) -> int {
    if (label.empty()) return -1;
    auto it = label_to_fd.find(label);
    if (it != label_to_fd.end()) return it->second;

    std::string dev_path = find_device_path(label);
    if (dev_path.empty()) {
      if (!quiet) {
        std::cerr << "No " << label << " device found, skipping events."
                  << std::endl;
      }
      label_to_fd[label] = -1;
      return -1;
    }

    int fd = fs.open_read_write(dev_path.c_str());
    if (fd < 0) {
      if (!quiet) {
        std::cerr << "Failed to open " << dev_path << " for write (try: sudo): ";
        std::perror(dev_path.c_str());
      }
      label_to_fd[label] = -1;
      return -1;
    }

    label_to_path[label] = dev_path;
    label_to_fd[label] = fd;
    if (!quiet) {
      std::cout << "Playing back " << label << " to " << dev_path << std::endl;
    }
    return fd;
  };

  AsyncLogWriter log_writer;
  if (!quiet) {
    std::cout << "Playing back to input devices (Ctrl+C to stop)..." << std::endl;
    log_writer.start();
  }
  evdev::signal_install_sigint();

  std::string line;
  bool has_prev = false;
  long long prev_timestamp_us = 0;

  while (!evdev::signal_stop_requested() && std::getline(input, line)) {
    if (line.empty()) continue;

    std::string label = parse_label(line);
    long long timestamp_us = 0;
    unsigned short type = 0;
    unsigned short code = 0;
    int value = 0;

    if (!parse_event_line(line, &timestamp_us, &type, &code, &value)) {
      continue;
    }

    int fd = get_fd(label);
    if (fd < 0) continue;

    if (has_prev && timestamp_us > prev_timestamp_us) {
      long long delta_us = timestamp_us - prev_timestamp_us;
      std::this_thread::sleep_for(std::chrono::microseconds(delta_us));
    }
    prev_timestamp_us = timestamp_us;
    has_prev = true;

    if (!write_event(&fs, fd, type, code, value)) {
      std::perror("Failed to write event");
      for (const auto& p : label_to_fd) {
        if (p.second >= 0) fs.close_fd(p.second);
      }
      evdev::signal_restore_sigint();
      return 1;
    }

    if (type != EV_SYN) {
      bool needs_mt_report =
          (type == EV_ABS && (code == ABS_MT_POSITION_Y ||
                              (code == ABS_MT_TRACKING_ID && value == -1)));
      if (needs_mt_report) {
        if (!write_event(&fs, fd, EV_SYN, SYN_MT_REPORT, 0)) {
          std::perror("Failed to write SYN_MT_REPORT");
          for (const auto& p : label_to_fd) {
            if (p.second >= 0) fs.close_fd(p.second);
          }
          evdev::signal_restore_sigint();
          return 1;
        }
      }
      if (!write_event(&fs, fd, EV_SYN, SYN_REPORT, 0)) {
        std::perror("Failed to write SYN_REPORT");
        for (const auto& p : label_to_fd) {
          if (p.second >= 0) fs.close_fd(p.second);
        }
        evdev::signal_restore_sigint();
        return 1;
      }
    }

    if (!quiet) {
      log_writer.push(line);
    }
  }

  log_writer.stop();

  for (const auto& p : label_to_fd) {
    if (p.second >= 0) fs.close_fd(p.second);
  }
  evdev::signal_restore_sigint();
  return 0;
}
