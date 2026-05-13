#include "evrp/server/impl/server/record.h"

#include <sys/time.h>

#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#include "evrp/device/api/inputlistener.h"
#include "evrp/sdk/types.h"
#include "evrp/sdk/eventformat.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/scopeguard.h"
#include "evrp/sdk/setting/isetting.h"

Record::Record(std::shared_ptr<ISetting> setting,
               evrp::device::api::IInputListener *listener,
               IEnhancedFileSystem *fs)
    : listener_(listener), fs_(fs) {
  if (!setting) {
    return;
  }
  logLevel_ = setting->get("logLevel", logging::LogLevel::Info);
  kinds_ =
      setting->get("kinds", std::vector<evrp::device::api::DeviceKind>{});
  device_ = setting->get<std::string>("device", {});
  outputPath_ = setting->get<std::string>("outputPath", {});
  keyboardCtrlCFilterMode_ = keyboardCtrlCFilterModeFromLabel(
      setting->get<std::string>("keyboardCtrlCFilter", "ending"));
}

Record::Record(std::shared_ptr<ISetting> setting, const evrp::Ioc &ioc)
    : Record(std::move(setting), ioc.get<evrp::device::api::IInputListener>(),
             ioc.get<IEnhancedFileSystem>()) {}

int Record::run() {
  logService->setLevel(logLevel_);
  if (!listener_) {
    logError("Record has no IInputListener.");
    return 1;
  }
  if (!fs_) {
    logError("Record has no IEnhancedFileSystem.");
    return 1;
  }
  if (!listener_->startListening(kinds_)) {
    logError(
        "startListening failed. Is evrp-device running on {} with input "
        "devices available?",
        device_);
    return 1;
  }
  evrp::sdk::ScopeGuard stopListening{[&]() {
    listener_->cancelListening();
  }};

  int outFd =
      fs_->openFd(outputPath_, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (outFd < 0) {
    int err = errno;
    logError("Failed to open output file {}: {}", outputPath_, strerror(err));
    return 1;
  }
  const bool ownOutputFd = !outputPath_.empty();
  evrp::sdk::ScopeGuard closeOutputFd{[fs = fs_, fd = outFd,
                                       own = ownOutputFd]() {
    if (own && fd >= 0) {
      fs->closeFd(fd);
    }
  }};

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
  logInfo("Recording from evrp-device at {} (Ctrl+C to stop)", device_);

  while (!sigint.stopRequested() && writeOk) {
    if (!listener_->waitForInputEvent(kWaitMs)) {
      continue;
    }
    std::vector<evrp::device::api::InputEvent> batch =
        listener_->readInputEvents();
    for (const auto &ine : batch) {
      if (ine.device == evrp::device::api::DeviceKind::kKeyboard &&
          keyboardCtrlCFilterMode_ != KeyboardCtrlCFilterMode::kOff) {
        Event ev = {static_cast<long>(ine.timeSec),
                    static_cast<long>(ine.timeUsec),
                    static_cast<unsigned short>(ine.type),
                    static_cast<unsigned short>(ine.code), ine.value};
        std::vector<Event> emitted;
        processKeyboardEventWithCtrlFilter(ev, keyboardCtrlCFilterMode_,
                                           &keyboardFilterState_, &emitted);
        for (const Event &e : emitted) {
          evrp::device::api::InputEvent out = {};
          out.device = evrp::device::api::DeviceKind::kKeyboard;
          out.timeSec = e.sec;
          out.timeUsec = e.usec;
          out.type = static_cast<uint32_t>(e.type);
          out.code = static_cast<uint32_t>(e.code);
          out.value = e.value;
          writeEventLine(out.device, out);
          if (!writeOk) {
            break;
          }
        }
      } else {
        writeEventLine(ine.device, ine);
      }
      if (!writeOk) {
        break;
      }
    }
  }

  if (writeOk && keyboardCtrlCFilterMode_ != KeyboardCtrlCFilterMode::kOff) {
    std::vector<Event> flushed;
    flushKeyboardEventFilter(keyboardCtrlCFilterMode_, &keyboardFilterState_,
                             &flushed);
    for (const Event &e : flushed) {
      evrp::device::api::InputEvent out = {};
      out.device = evrp::device::api::DeviceKind::kKeyboard;
      out.timeSec = e.sec;
      out.timeUsec = e.usec;
      out.type = static_cast<uint32_t>(e.type);
      out.code = static_cast<uint32_t>(e.code);
      out.value = e.value;
      writeEventLine(out.device, out);
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
