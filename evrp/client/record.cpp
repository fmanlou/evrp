#include "record.h"

#include <sys/time.h>

#include <cstdio>
#include <ostream>
#include <string>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/types.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "logger.h"

Record::Record(const RunOptions &options, const evrp::Ioc &ioc)
    : options_(options), ioc_(ioc) {}

int Record::run() {
  logService->setLevel(options_.logLevel);
  evrp::device::api::IInputListener *listener =
      ioc_.get<evrp::device::api::IInputListener>();
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
