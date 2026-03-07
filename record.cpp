#include "record.h"

#include "asynclogwriter.h"
#include "eventformat.h"
#include "evdev.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboarddevice.h"
#include "touchdevice.h"

#include <cstdio>
#include <iostream>
#include <linux/input-event-codes.h>

Record::Record(const run_options& options) : options_(options) {}

std::vector<RecordTarget> Record::collect_targets() {
  std::vector<RecordTarget> result;
  for (const auto& kind : options_.kinds) {
    std::string path = find_device_path(kind);
    if (path.empty()) {
      std::cout << "No " << kind << " detected. Try running with sudo."
                << std::endl;
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
  for (const auto& t : targets_) fs_.close_fd(t.fd);
}

void Record::record_events() {
  if (targets_.empty()) return;

  AsyncLogWriter log_writer;
  log_writer.start();

  std::vector<int> fds;
  fds.reserve(targets_.size());
  for (const auto& t : targets_) {
    log_writer.push("Recording " + t.label + " from " + t.path);
    fds.push_back(t.fd);
  }
  log_writer.push("(Ctrl+C to stop)");

  SigintGuard sigint;
  std::ostream& event_out = fs_.output_stream();
  bool log_events_to_console = !options_.quiet;
  auto write_line = [&](const std::string& line) {
    event_out << line << "\n";
    if (log_events_to_console) log_writer.push(line);
  };
  auto write_newline = [&]() {
    event_out << "\n";
    if (log_events_to_console) log_writer.push("");
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
        const auto& ev = events[j];
        if (ev.type == EV_SYN) continue;

        if (targets_[i].label == "keyboard") {
          std::vector<Event> emitted_events;
          process_keyboard_event_with_ctrl_filter(ev, &keyboard_states[i],
                                                  &emitted_events);
          for (const auto& out_ev : emitted_events) {
            write_line(format_event_line(targets_[i].label, out_ev));
          }
          continue;
        }

        if (targets_[i].label == "touchpad") {
          touch_segment_state& touch_state = touch_states[i];
          touch_segment_decision decision =
              process_touch_event_for_segment(ev, &touch_state);
          if (decision.emit_break_before_event) {
            write_newline();
          }

          write_line(format_event_line(targets_[i].label, ev));
          if (decision.emit_break_after_event) {
            write_newline();
          }
          continue;
        }

        write_line(format_event_line(targets_[i].label, ev));
      }
    }
  }

  for (size_t i = 0; i < touch_states.size(); ++i) {
    if (touch_states[i].pending_segment_break) {
      write_newline();
      touch_states[i].pending_segment_break = false;
    }
  }

  event_out.flush();
  log_writer.stop();
}

int Record::run() {
  targets_ = collect_targets();

  if (targets_.empty()) {
    std::cout << "No devices to record." << std::endl;
    return 1;
  }

  if (!fs_.open_output(options_.output_path)) {
    std::cerr << fs_.error_message() << std::endl;
    close_targets();
    return 1;
  }

  record_events();
  close_targets();
  return 0;
}
