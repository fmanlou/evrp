#include "record.h"

#include "asynclogwriter.h"
#include "evdev/evdev.h"
#include "filesystem/filesystem.h"
#include "inputdevice.h"
#include "keyboarddevice.h"
#include "touchdevice.h"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <linux/input-event-codes.h>
#include <sstream>

namespace {

static std::string event_type_name(unsigned short type) {
  switch (type) {
    case EV_SYN:
      return "EV_SYN";
    case EV_KEY:
      return "EV_KEY";
    case EV_REL:
      return "EV_REL";
    case EV_ABS:
      return "EV_ABS";
    case EV_MSC:
      return "EV_MSC";
    default:
      return "EV_UNKNOWN";
  }
}

static std::string event_code_name(unsigned short type, unsigned short code) {
  if (type == EV_MSC) {
    switch (code) {
      case MSC_SCAN:
        return "MSC_SCAN";
      case MSC_TIMESTAMP:
        return "MSC_TIMESTAMP";
      default:
        return "";
    }
  }
  if (type == EV_SYN) {
    switch (code) {
      case SYN_REPORT:
        return "SYN_REPORT";
      case SYN_MT_REPORT:
        return "SYN_MT_REPORT";
      default:
        return "";
    }
  }
  return "";
}

static std::string format_event_line(const std::string& label,
                                     const evdev::Event& ev) {
  std::ostringstream oss;
  std::string code_name = event_code_name(ev.type, ev.code);
  oss << "[" << label << "] " << ev.sec << "." << ev.usec << " type=" << ev.type
      << "(" << event_type_name(ev.type) << ")"
      << " code=" << ev.code;
  if (!code_name.empty()) {
    oss << "(" << code_name << ")";
  }
  oss << " value=" << ev.value;
  if (label == "keyboard") {
    if (ev.type == EV_KEY) {
      oss << " // key=" << keyboard_key_name_from_code(ev.code)
          << " action=" << keyboard_key_action_from_value(ev.value);
    } else {
      oss << " // key=N/A action=non-key-event";
    }
  }
  return oss.str();
}

static std::string find_device_path(const std::string& kind) {
  if (kind == "touchpad") return find_first_touchpad();
  if (kind == "mouse") return find_first_mouse();
  return find_first_keyboard();
}

}  // namespace

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
  evdev::signal_install_sigint();

  std::vector<int> fds;
  fds.reserve(targets_.size());
  for (const auto& t : targets_) {
    log_writer.push("Recording " + t.label + " from " + t.path);
    fds.push_back(t.fd);
  }
  log_writer.push("(Ctrl+C to stop)");

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

  evdev::Event events[64];
  bool ready[32];
  std::vector<keyboard_filter_state> keyboard_states(fds.size());
  std::vector<touch_segment_state> touch_states(fds.size());
  while (!evdev::signal_stop_requested()) {
    int ret = fs_.poll_fds(fds.data(), static_cast<int>(fds.size()), -1, ready);
    if (ret < 0) {
      if (evdev::errno_is_eintr() && evdev::signal_stop_requested()) break;
      std::perror("poll");
      break;
    }

    for (size_t i = 0; i < fds.size(); ++i) {
      if (!ready[i]) continue;

      int count = evdev::read_events(fds[i], events, 64);
      if (count < 0) {
        std::perror("read");
        break;
      }
      if (count == 0) continue;

      for (int j = 0; j < count; ++j) {
        const auto& ev = events[j];
        if (ev.type == EV_SYN) continue;

        if (targets_[i].label == "keyboard") {
          std::vector<evdev::Event> emitted_events;
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
  evdev::signal_restore_sigint();
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
