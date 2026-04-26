#include "record.h"

#include <linux/input-event-codes.h>
#include <sys/time.h>

#include <cstdio>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboarddevice.h"
#include "logger.h"
#include "touchdevice.h"

Record::Record(const RunOptions &options) : options_(options) {}

std::vector<RecordTarget> Record::collectTargets() {
  std::vector<RecordTarget> result;
  for (const auto &kind : options_.kinds) {
    const std::vector<std::string> paths = findAllDevicePaths(kind);
    if (paths.empty()) {
      logWarn("No {} detected. Try running with sudo.",
              evrp::device::api::deviceKindLabel(kind));
      continue;
    }
    for (const std::string &path : paths) {
      int fd = fs_.openReadOnly(path.c_str(), false);
      if (fd < 0) {
        std::perror(path.c_str());
        continue;
      }
      result.push_back({fd, kind, path});
    }
  }
  return result;
}

void Record::closeTargets() {
  for (const auto &t : targets_) fs_.closeFd(t.fd);
}

void Record::recordEvents() {
  if (targets_.empty()) return;

  logService->setLevel(options_.logLevel);

  std::vector<int> fds;
  fds.reserve(targets_.size());
  for (const auto &t : targets_) {
    logInfo("Recording {} from {}",
            evrp::device::api::deviceKindLabel(t.kind),
            t.path);
    fds.push_back(t.fd);
  }
  logInfo("(Ctrl+C to stop)");

  SigintGuard sigint;
  std::ostream &eventOut = fs_.outputStream();
  struct timeval sessionStart = {};
  gettimeofday(&sessionStart, nullptr);
  long long baselineUs = -1;
  long long lastTimestampUs = -1;
  auto writeLine = [&](const std::string &line) {
    eventOut << line << "\n";
    logDebug("{}", line);
  };
  auto writeEventLine = [&](evrp::device::api::DeviceKind device,
                            const Event &ev) {
    long long currentUs = ev.sec * 1000000LL + ev.usec;
    if (baselineUs < 0) {
      baselineUs = currentUs;
      long long sessionStart_us =
          sessionStart.tv_sec * 1000000LL + sessionStart.tv_usec;
      long long leadingUs = currentUs - sessionStart_us;
      if (leadingUs > 0) {
        writeLine(formatLeadingLine(leadingUs));
      }
    }
    lastTimestampUs = currentUs;
    long long deltaUs = currentUs - baselineUs;
    writeLine(formatEventLine(device, ev, deltaUs));
  };
  auto writeNewline = [&]() {
    eventOut << "\n";
    logDebug("");
  };

  Event events[64];
  bool ready[32];
  std::vector<keyboard_filter_state> keyboardStates(fds.size());
  std::vector<touch_segment_state> touchStates(fds.size());
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
          std::vector<Event> emittedEvents;
          processKeyboardEventWithCtrlFilter(ev, &keyboardStates[i],
                                                  &emittedEvents);
          for (const auto &outEv : emittedEvents) {
            writeEventLine(targets_[i].kind, outEv);
          }
          continue;
        }

        if (targets_[i].kind == evrp::device::api::DeviceKind::kTouchpad ||
            targets_[i].kind == evrp::device::api::DeviceKind::kTouchscreen) {
          touch_segment_state &touchState = touchStates[i];
          touch_segment_decision decision =
              processTouchEventForSegment(ev, &touchState);
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

  for (size_t i = 0; i < touchStates.size(); ++i) {
    if (touchStates[i].pending_segment_break) {
      writeNewline();
      touchStates[i].pending_segment_break = false;
    }
  }

  if (lastTimestampUs >= 0) {
    struct timeval t_end = {};
    gettimeofday(&t_end, nullptr);
    long long endUs = t_end.tv_sec * 1000000LL + t_end.tv_usec;
    long long trailingUs = endUs - lastTimestampUs;
    if (trailingUs > 0) {
      writeLine(formatTrailingLine(trailingUs));
    }
  }

  eventOut.flush();
}

int Record::runWithIoc(const evrp::Ioc &ioc) {
  logService->setLevel(options_.logLevel);
  evrp::device::api::IInputListener *listener =
      ioc.get<evrp::device::api::IInputListener>();
  if (!listener) {
    logError("Ioc has no IInputListener.");
    return 1;
  }
  if (!listener->startListening(options_.kinds)) {
    logError(
        "startListening failed. Is evrp-device running on {} with input "
        "devices available?",
        options_.device);
    return 1;
  }
  struct StopListenGuard {
    evrp::device::api::IInputListener *l;
    ~StopListenGuard() {
      if (l) {
        l->cancelListening();
      }
    }
  } stopGuard{listener};

  if (!fs_.openOutput(options_.outputPath)) {
    logError("{}", fs_.errorMessage());
    return 1;
  }

  std::ostream &eventOut = fs_.outputStream();
  struct timeval sessionStart = {};
  gettimeofday(&sessionStart, nullptr);
  long long baselineUs = -1;
  long long lastClientEventUs = -1;

  SigintGuard sigint;
  auto writeLine = [&](const std::string &line) {
    eventOut << line << "\n";
    logDebug("{}", line);
  };
  auto writeEventLine = [&](evrp::device::api::DeviceKind device,
                            const evrp::device::api::InputEvent &ine) {
    long long currentUs =
        ine.timeSec * 1000000LL + ine.timeUsec;
    if (baselineUs < 0) {
      baselineUs = currentUs;
      struct timeval now = {};
      gettimeofday(&now, nullptr);
      long long sessionStart_us =
          sessionStart.tv_sec * 1000000LL + sessionStart.tv_usec;
      long long now_us = now.tv_sec * 1000000LL + now.tv_usec;
      long long leadingUs = now_us - sessionStart_us;
      if (leadingUs > 0) {
        writeLine(formatLeadingLine(leadingUs));
      }
    }
    long long deltaUs = currentUs - baselineUs;
    Event ev = {static_cast<long>(ine.timeSec), static_cast<long>(ine.timeUsec),
                static_cast<unsigned short>(ine.type),
                static_cast<unsigned short>(ine.code), ine.value};
    writeLine(formatEventLine(device, ev, deltaUs));
    struct timeval now = {};
    gettimeofday(&now, nullptr);
    lastClientEventUs = now.tv_sec * 1000000LL + now.tv_usec;
  };
  constexpr int kWaitMs = 500;
  logInfo("Recording from evrp-device at {} (Ctrl+C to stop)",
          options_.device);

  while (!sigint.stopRequested()) {
    if (!listener->waitForInputEvent(kWaitMs)) {
      continue;
    }
    std::vector<evrp::device::api::InputEvent> batch =
        listener->readInputEvents();
    for (const auto &ine : batch) {
      writeEventLine(ine.device, ine);
    }
  }

  if (lastClientEventUs >= 0) {
    struct timeval t_end = {};
    gettimeofday(&t_end, nullptr);
    long long endUs = t_end.tv_sec * 1000000LL + t_end.tv_usec;
    long long trailingUs = endUs - lastClientEventUs;
    if (trailingUs > 0) {
      writeLine(formatTrailingLine(trailingUs));
    }
  }

  eventOut.flush();
  return 0;
}

int Record::run() {
  logService->setLevel(options_.logLevel);
  targets_ = collectTargets();

  if (targets_.empty()) {
    logError("No devices to record.");
    return 1;
  }

  if (!fs_.openOutput(options_.outputPath)) {
    logError("{}", fs_.errorMessage());
    closeTargets();
    return 1;
  }

  recordEvents();
  closeTargets();
  return 0;
}
