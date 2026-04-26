#include "inputeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>

#include <cstdio>
#include <vector>

#include "cursor/cursorpos.h"
#include "evrp/device/api/playback.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboardeventwriter.h"
#include "logger.h"
#include "mouse/mouseeventwriter.h"

namespace api = evrp::device::api;

InputEventWriter::InputEventWriter(FileSystem *fs)
    : fs_(fs), keyboardWriter_(this), mouseWriter_(this, gCursor) {}

InputEventWriter::~InputEventWriter() {
  for (const auto &p : kindToFd_) {
    if (p.second >= 0) fs_->closeFd(p.second);
  }
}

void InputEventWriter::setRemotePlayback(api::IPlayback *playback) {
  remotePlayback_ = playback;
  remoteTimelineUs_ = 0;
  remoteHasWall_ = false;
}

bool InputEventWriter::flushRemoteBatch(std::vector<api::InputEvent> batch) {
  if (!remotePlayback_ || batch.empty()) {
    return true;
  }
  const auto wall = std::chrono::steady_clock::now();
  if (remoteHasWall_) {
    const auto gap = std::chrono::duration_cast<std::chrono::microseconds>(
        wall - remoteLastWall_);
    const int64_t g = gap.count();
    if (g > 0) {
      remoteTimelineUs_ += g;
    }
  }
  remoteLastWall_ = wall;
  remoteHasWall_ = true;
  for (auto &e : batch) {
    e.timeSec = remoteTimelineUs_ / 1000000LL;
    e.timeUsec = remoteTimelineUs_ % 1000000LL;
    remoteTimelineUs_ += 1;
  }
  api::OperationResult up;
  if (!remotePlayback_->upload(batch, &up) || up.code != 0) {
    logError("Remote inject: upload failed (code={}): {}", up.code, up.message);
    return false;
  }
  api::OperationResult play;
  if (!remotePlayback_->playback(&play) || play.code != 0) {
    logError("Remote inject: playback failed (code={}): {}", play.code,
             play.message);
    return false;
  }
  return true;
}

bool InputEventWriter::writeRawRemote(api::DeviceKind device,
                                      unsigned short type, unsigned short code,
                                      int value) {
  std::vector<api::InputEvent> batch;
  auto push = [&](unsigned short t, unsigned short c, int v) {
    api::InputEvent e;
    e.device = device;
    e.type = static_cast<uint32_t>(t);
    e.code = static_cast<uint32_t>(c);
    e.value = v;
    batch.push_back(e);
  };
  push(type, code, value);
  if (type != EV_SYN) {
    const bool needs_mt =
        (type == EV_ABS &&
         (code == ABS_MT_POSITION_Y ||
          (code == ABS_MT_TRACKING_ID && value == -1)));
    if (needs_mt) {
      push(EV_SYN, SYN_MT_REPORT, 0);
    }
    push(EV_SYN, SYN_REPORT, 0);
  }
  return flushRemoteBatch(std::move(batch));
}

int InputEventWriter::getFd(api::DeviceKind device) {
  if (device == api::DeviceKind::kUnspecified) return -1;
  auto it = kindToFd_.find(device);
  if (it != kindToFd_.end()) return it->second;

  const std::vector<std::string> paths = findAllDevicePaths(device);
  if (paths.empty()) {
    logWarn("No {} device found, skipping events.",
            api::deviceKindLabel(device));
    kindToFd_[device] = -1;
    return -1;
  }
  for (const std::string& devPath : paths) {
    int fd = fs_->openReadWrite(devPath.c_str());
    if (fd >= 0) {
      kindToFd_[device] = fd;
      logInfo("Playing back {} to {}", api::deviceKindLabel(device), devPath);
      return fd;
    }
    std::perror(devPath.c_str());
  }
  logWarn(
      "Failed to open any of {} {} device path(s) for write (try: sudo or "
      "input group)",
      paths.size(),
      api::deviceKindLabel(device));
  kindToFd_[device] = -1;
  return -1;
}

bool InputEventWriter::write(api::DeviceKind device, unsigned short type,
                             unsigned short code, int value) {
  if (device == api::DeviceKind::kKeyboard) {
    return keyboardWriter_.write(type, code, value);
  }
  if (device == api::DeviceKind::kMouse) {
    return mouseWriter_.write(type, code, value);
  }
  return writeRaw(device, type, code, value);
}

bool InputEventWriter::writeRaw(api::DeviceKind device, unsigned short type,
                                unsigned short code, int value) {
  if (remotePlayback_) {
    if (type != EV_SYN) {
      struct timeval tv;
      gettimeofday(&tv, nullptr);
      Event ev = {tv.tv_sec, tv.tv_usec, type, code, value};
      logDebug("{}", formatEventLine(device, ev, 0));
    }
    return writeRawRemote(device, type, code, value);
  }
  int fd = getFd(device);
  if (fd < 0) return true;  
  if (type != EV_SYN) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    Event ev = {tv.tv_sec, tv.tv_usec, type, code, value};
    logDebug("{}", formatEventLine(device, ev, 0));
  }
  return writeEventWithSync(fd, type, code, value);
}

bool InputEventWriter::writeEvent(int fd, unsigned short type,
                                  unsigned short code, int value) {
  struct input_event ev = {};
  gettimeofday(&ev.time, nullptr);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  long n = fs_->writeFd(fd, &ev, sizeof(ev));
  return n == static_cast<long>(sizeof(ev));
}

bool InputEventWriter::writeEventWithSync(int fd, unsigned short type,
                                          unsigned short code, int value) {
  if (!writeEvent(fd, type, code, value)) {
    std::perror("Failed to write event");
    return false;
  }
  if (type != EV_SYN) {
    bool needs_mt =
        (type == EV_ABS && (code == ABS_MT_POSITION_Y ||
                            (code == ABS_MT_TRACKING_ID && value == -1)));
    if (needs_mt && !writeEvent(fd, EV_SYN, SYN_MT_REPORT, 0)) {
      std::perror("Failed to write SYN_MT_REPORT");
      return false;
    }
    if (!writeEvent(fd, EV_SYN, SYN_REPORT, 0)) {
      std::perror("Failed to write SYN_REPORT");
      return false;
    }
  }
  return true;
}
