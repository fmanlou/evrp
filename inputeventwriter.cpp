#include "inputeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>

#include <cstdio>
#include <sstream>

#include "cursor/cursorpos.h"
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

int InputEventWriter::getFd(api::DeviceKind device) {
  if (device == api::DeviceKind::kUnspecified) return -1;
  auto it = kindToFd_.find(device);
  if (it != kindToFd_.end()) return it->second;

  std::string devPath = findDevicePath(device);
  if (devPath.empty()) {
    logWarn(std::string("No ") + api::deviceKindLabel(device) +
            " device found, skipping events.");
    kindToFd_[device] = -1;
    return -1;
  }

  int fd = fs_->openReadWrite(devPath.c_str());
  if (fd < 0) {
    std::ostringstream oss;
    oss << "Failed to open " << devPath << " for write (try: sudo)";
    logWarn(oss.str());
    std::perror(devPath.c_str());
    kindToFd_[device] = -1;
    return -1;
  }

  kindToFd_[device] = fd;
  logInfo(std::string("Playing back ") + api::deviceKindLabel(device) + " to " +
          devPath);
  return fd;
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
    logDebug(formatEventLine(device, ev, 0));
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
