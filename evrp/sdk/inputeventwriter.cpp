#include "evrp/sdk/inputeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>

#include <fcntl.h>

#include <cstdio>

#include "evrp/sdk/cursor/cursorpos.h"
#include "evrp/sdk/evdev.h"
#include "evrp/sdk/eventformat.h"
#include "evrp/sdk/filesystem/enhancedfilesystem.h"
#include "evrp/sdk/inputdevice.h"
#include "evrp/sdk/keyboard/keyboardeventwriter.h"
#include "evrp/sdk/logger.h"
#include "evrp/sdk/mouse/mouseeventwriter.h"

namespace api = evrp::device::api;

InputEventWriter::InputEventWriter(IEnhancedFileSystem *fs)
    : fs_(fs), keyboardWriter_(this), mouseWriter_(this, gCursor) {}

InputEventWriter::~InputEventWriter() {
  for (const auto &p : kindToFd_) {
    if (p.second >= 0) fs_->closeFd(p.second);
  }
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
    int fd = fs_->openFd(devPath, O_RDWR, 0);
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
