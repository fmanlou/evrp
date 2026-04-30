#include "record.h"

#include <sys/time.h>

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/evdev.h"
#include "evrp/sdk/eventformat.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/logger.h"

Record::Record(MemorySetting parsed, evrp::device::api::IInputListener *listener,
               IEnhancedFileSystem *fs)
    : parsed_(std::move(parsed)), listener_(listener), fs_(fs) {}

Record::Record(MemorySetting parsed, const evrp::Ioc &ioc)
    : Record(std::move(parsed), ioc.get<evrp::device::api::IInputListener>(),
             ioc.get<IEnhancedFileSystem>()) {}

int Record::run() {
  logService->setLevel(parsed_.get("logLevel", logging::LogLevel::Info));
  if (!listener_) {
    logError("Record has no IInputListener.");
    return 1;
  }
  if (!fs_) {
    logError("Record has no IEnhancedFileSystem.");
    return 1;
  }
  const std::vector<evrp::device::api::DeviceKind> kinds =
      parsed_.get("kinds", std::vector<evrp::device::api::DeviceKind>{});
  if (!listener_->startListening(kinds)) {
    logError(
        "startListening failed. Is evrp-device running on {} with input "
        "devices available?",
        parsed_.get<std::string>("device", {}));
    return 1;
  }
  struct StopListenGuard {
    evrp::device::api::IInputListener *l;
    ~StopListenGuard() {
      if (l) {
        l->cancelListening();
      }
    }
  } stopGuard{listener_};

  const std::string outputPath = parsed_.get<std::string>("outputPath", {});
  int outFd = fs_->openFd(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (outFd < 0) {
    int err = errno;
    logError("Failed to open output file {}: {}", outputPath, strerror(err));
    return 1;
  }
  const bool ownOutputFd = !outputPath.empty();
  struct OutputFdGuard {
    IEnhancedFileSystem *fs;
    int fd;
    bool own;
    ~OutputFdGuard() {
      if (own && fd >= 0) {
        fs->closeFd(fd);
      }
    }
  } outputFdGuard{fs_, outFd, ownOutputFd};

  struct timeval sessionStart = {};
  gettimeofday(&sessionStart, nullptr);
  long long baselineUs = -1;
  long long lastClientEventUs = -1;
  bool writeOk = true;

  SigintGuard sigint;
  auto writeLine = [&](const std::string &line) {
    if (!writeOk) {
      return;
    }
    if (!fs_->writeOutput(outFd, line) || !fs_->writeOutput(outFd, "\n")) {
      logError("Write to recording output failed.");
      writeOk = false;
      return;
    }
    logDebug("{}", line);
  };
  auto writeEventLine = [&](evrp::device::api::DeviceKind device,
                            const evrp::device::api::InputEvent &ine) {
    if (!writeOk) {
      return;
    }
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
    if (!writeOk) {
      return;
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
          parsed_.get<std::string>("device", {}));

  while (!sigint.stopRequested() && writeOk) {
    if (!listener_->waitForInputEvent(kWaitMs)) {
      continue;
    }
    std::vector<evrp::device::api::InputEvent> batch =
        listener_->readInputEvents();
    for (const auto &ine : batch) {
      writeEventLine(ine.device, ine);
      if (!writeOk) {
        break;
      }
    }
  }

  if (writeOk && lastClientEventUs >= 0) {
    struct timeval t_end = {};
    gettimeofday(&t_end, nullptr);
    long long endUs = t_end.tv_sec * 1000000LL + t_end.tv_usec;
    long long trailingUs = endUs - lastClientEventUs;
    if (trailingUs > 0) {
      writeLine(formatTrailingLine(trailingUs));
    }
  }

  if (!writeOk) {
    return 1;
  }
  if (!fs_->flushFd(outFd)) {
    logError("Flush recording output failed.");
    return 1;
  }
  return 0;
}
