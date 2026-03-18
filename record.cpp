#include "record.h"

#include <linux/input-event-codes.h>
#include <sys/time.h>

#include <cstdio>

#include "deviceid.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboarddevice.h"
#include "logger.h"
#include "touchdevice.h"

Record::Record(const run_options &options) : options_(options) {}

std::vector<RecordTarget> Record::collect_targets() {
  std::vector<RecordTarget> result;
  for (const auto &kind : options_.kinds) {
    std::string path = find_device_path(kind);
    if (path.empty()) {
      log_warn("No " + std::string(device_label(kind)) +
               " detected. Try running with sudo.");
      continue;
    }

    int fd = fs_.open_read_only(path.c_str(), false);
    if (fd < 0) {
      std::perror(path.c_str());
      continue;
    }
    result.push_back({fd, kind, path});
  }
  return result;
}

void Record::close_targets() {
  for (const auto &t : targets_) fs_.close_fd(t.fd);
}

void Record::record_events() {
  if (targets_.empty()) return;

  g_logger->set_level(options_.log_level);

  std::vector<int> fds;
  fds.reserve(targets_.size());
  for (const auto &t : targets_) {
    log_info("Recording " + std::string(device_label(t.id)) + " from " +
             t.path);
    fds.push_back(t.fd);
  }
  log_info("(Ctrl+C to stop)");

  SigintGuard sigint;
  std::ostream &event_out = fs_.output_stream();
  struct timeval session_start = {};
  gettimeofday(&session_start, nullptr);
  long long baseline_us = -1;
  long long last_timestamp_us = -1;
  auto write_line = [&](const std::string &line) {
    event_out << line << "\n";
    log_debug(line);
  };
  auto write_event_line = [&](DeviceId id, const Event &ev) {
    long long current_us = ev.sec * 1000000LL + ev.usec;
    if (baseline_us < 0) {
      baseline_us = current_us;
      long long session_start_us =
          session_start.tv_sec * 1000000LL + session_start.tv_usec;
      long long leading_us = current_us - session_start_us;
      if (leading_us > 0) {
        write_line(format_leading_line(leading_us));
      }
    }
    last_timestamp_us = current_us;
    long long delta_us = current_us - baseline_us;
    write_line(format_event_line(id, ev, delta_us));
  };
  auto write_newline = [&]() {
    event_out << "\n";
    log_debug("");
  };

  Event events[64];
  bool ready[32];
  std::vector<keyboard_filter_state> keyboard_states(fds.size());
  std::vector<touch_segment_state> touch_states(fds.size());
  while (!sigint.stop_requested()) {
    int ret = fs_.poll_fds(fds.data(), static_cast<int>(fds.size()), -1, ready);
    if (ret < 0) {
      if (errno_is_eintr() && sigint.stop_requested()) break;
      std::perror("poll");
      break;
    }

    for (size_t i = 0; i < fds.size(); ++i) {
      if (!ready[i]) continue;

      int count = read_events(fds[i], events, 64);
      if (count < 0) {
        std::perror("read");
        break;
      }
      if (count == 0) continue;

      for (int j = 0; j < count; ++j) {
        const auto &ev = events[j];
        if (ev.type == EV_SYN) continue;

        if (targets_[i].id == DeviceId::Keyboard) {
          std::vector<Event> emitted_events;
          process_keyboard_event_with_ctrl_filter(ev, &keyboard_states[i],
                                                  &emitted_events);
          for (const auto &out_ev : emitted_events) {
            write_event_line(targets_[i].id, out_ev);
          }
          continue;
        }

        if (targets_[i].id == DeviceId::Touchpad ||
            targets_[i].id == DeviceId::Touchscreen) {
          touch_segment_state &touch_state = touch_states[i];
          touch_segment_decision decision =
              process_touch_event_for_segment(ev, &touch_state);
          if (decision.emit_break_before_event) {
            write_newline();
          }

          write_event_line(targets_[i].id, ev);
          if (decision.emit_break_after_event) {
            write_newline();
          }
          continue;
        }

        write_event_line(targets_[i].id, ev);
      }
    }
  }

  for (size_t i = 0; i < touch_states.size(); ++i) {
    if (touch_states[i].pending_segment_break) {
      write_newline();
      touch_states[i].pending_segment_break = false;
    }
  }

  if (last_timestamp_us >= 0) {
    struct timeval t_end = {};
    gettimeofday(&t_end, nullptr);
    long long end_us = t_end.tv_sec * 1000000LL + t_end.tv_usec;
    long long trailing_us = end_us - last_timestamp_us;
    if (trailing_us > 0) {
      write_line(format_trailing_line(trailing_us));
    }
  }

  event_out.flush();
}

int Record::run() {
  g_logger->set_level(options_.log_level);
  targets_ = collect_targets();

  if (targets_.empty()) {
    log_error("No devices to record.");
    return 1;
  }

  if (!fs_.open_output(options_.output_path)) {
    log_error(fs_.error_message());
    close_targets();
    return 1;
  }

  record_events();
  close_targets();
  return 0;
}
