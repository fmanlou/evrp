#include "record.h"

#include <linux/input-event-codes.h>
#include <sys/time.h>

#include <cstdio>

#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboarddevice.h"
#include "logger.h"
#include "touchdevice.h"

Record::Record(const run_options &options) : options_(options) {}

std::vector<RecordTarget> Record::collectTargets() {
  std::vector<RecordTarget> result;
  for (const auto &kind : options_.kinds) {
    std::string path = findDevicePath(kind);
    if (path.empty()) {
      logWarn("No " + std::string(evrp::device::api::deviceKindLabel(kind)) +
               " detected. Try running with sudo.");
      continue;
    }

    int fd = fs_.openReadOnly(path.c_str(), false);
    if (fd < 0) {
      std::perror(path.c_str());
      continue;
    }
    result.push_back({fd, kind, path});
  }
  return result;
}

void Record::closeTargets() {
  for (const auto &t : targets_) fs_.closeFd(t.fd);
}

void Record::recordEvents() {
  if (targets_.empty()) return;

  g_logger->setLevel(options_.log_level);

  std::vector<int> fds;
  fds.reserve(targets_.size());
  for (const auto &t : targets_) {
    logInfo("Recording " +
            std::string(evrp::device::api::deviceKindLabel(t.kind)) + " from " +
            t.path);
    fds.push_back(t.fd);
  }
  logInfo("(Ctrl+C to stop)");

  SigintGuard sigint;
  std::ostream &event_out = fs_.outputStream();
  struct timeval session_start = {};
  gettimeofday(&session_start, nullptr);
  long long baseline_us = -1;
  long long last_timestamp_us = -1;
  auto writeLine = [&](const std::string &line) {
    event_out << line << "\n";
    logDebug(line);
  };
  auto writeEventLine = [&](evrp::device::api::DeviceKind device,
                            const Event &ev) {
    long long current_us = ev.sec * 1000000LL + ev.usec;
    if (baseline_us < 0) {
      baseline_us = current_us;
      long long session_start_us =
          session_start.tv_sec * 1000000LL + session_start.tv_usec;
      long long leading_us = current_us - session_start_us;
      if (leading_us > 0) {
        writeLine(formatLeadingLine(leading_us));
      }
    }
    last_timestamp_us = current_us;
    long long delta_us = current_us - baseline_us;
    writeLine(formatEventLine(device, ev, delta_us));
  };
  auto writeNewline = [&]() {
    event_out << "\n";
    logDebug("");
  };

  Event events[64];
  bool ready[32];
  std::vector<keyboard_filter_state> keyboard_states(fds.size());
  std::vector<touch_segment_state> touch_states(fds.size());
  while (!sigint.stopRequested()) {
    int ret = fs_.pollFds(fds.data(), static_cast<int>(fds.size()), -1, ready);
    if (ret < 0) {
      if (errnoIsEintr() && sigint.stopRequested()) break;
      std::perror("poll");
      break;
    }

    for (size_t i = 0; i < fds.size(); ++i) {
      if (!ready[i]) continue;

      int count = readEvents(fds[i], events, 64);
      if (count < 0) {
        std::perror("read");
        break;
      }
      if (count == 0) continue;

      for (int j = 0; j < count; ++j) {
        const auto &ev = events[j];
        if (ev.type == EV_SYN) continue;

        if (targets_[i].kind == evrp::device::api::DeviceKind::kKeyboard) {
          std::vector<Event> emitted_events;
          processKeyboardEventWithCtrlFilter(ev, &keyboard_states[i],
                                                  &emitted_events);
          for (const auto &out_ev : emitted_events) {
            writeEventLine(targets_[i].kind, out_ev);
          }
          continue;
        }

        if (targets_[i].kind == evrp::device::api::DeviceKind::kTouchpad ||
            targets_[i].kind == evrp::device::api::DeviceKind::kTouchscreen) {
          touch_segment_state &touch_state = touch_states[i];
          touch_segment_decision decision =
              processTouchEventForSegment(ev, &touch_state);
          if (decision.emit_break_before_event) {
            writeNewline();
          }

          writeEventLine(targets_[i].kind, ev);
          if (decision.emit_break_after_event) {
            writeNewline();
          }
          continue;
        }

        writeEventLine(targets_[i].kind, ev);
      }
    }
  }

  for (size_t i = 0; i < touch_states.size(); ++i) {
    if (touch_states[i].pending_segment_break) {
      writeNewline();
      touch_states[i].pending_segment_break = false;
    }
  }

  if (last_timestamp_us >= 0) {
    struct timeval t_end = {};
    gettimeofday(&t_end, nullptr);
    long long end_us = t_end.tv_sec * 1000000LL + t_end.tv_usec;
    long long trailing_us = end_us - last_timestamp_us;
    if (trailing_us > 0) {
      writeLine(formatTrailingLine(trailing_us));
    }
  }

  event_out.flush();
}

int Record::run() {
  g_logger->setLevel(options_.log_level);
  targets_ = collectTargets();

  if (targets_.empty()) {
    logError("No devices to record.");
    return 1;
  }

  if (!fs_.openOutput(options_.output_path)) {
    logError(fs_.errorMessage());
    closeTargets();
    return 1;
  }

  recordEvents();
  closeTargets();
  return 0;
}
