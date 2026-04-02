#include "inputeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>

#include <cstdio>
#include <sstream>

#include "cursor/cursorpos.h"
#include "deviceid.h"
#include "evdev.h"
#include "eventformat.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboardeventwriter.h"
#include "logger.h"
#include "mouse/mouseeventwriter.h"

InputEventWriter::InputEventWriter(FileSystem *fs)
    : fs_(fs), keyboard_writer_(this), mouse_writer_(this, g_cursor) {}

InputEventWriter::~InputEventWriter() {
  for (const auto &p : id_to_fd_) {
    if (p.second >= 0) fs_->closeFd(p.second);
  }
}

int InputEventWriter::getFd(DeviceId id) {
  if (id == DeviceId::Unknown) return -1;
  auto it = id_to_fd_.find(id);
  if (it != id_to_fd_.end()) return it->second;

  std::string dev_path = findDevicePath(id);
  if (dev_path.empty()) {
    logWarn(std::string("No ") + deviceLabel(id) +
             " device found, skipping events.");
    id_to_fd_[id] = -1;
    return -1;
  }

  int fd = fs_->openReadWrite(dev_path.c_str());
  if (fd < 0) {
    std::ostringstream oss;
    oss << "Failed to open " << dev_path << " for write (try: sudo)";
    logWarn(oss.str());
    std::perror(dev_path.c_str());
    id_to_fd_[id] = -1;
    return -1;
  }

  id_to_fd_[id] = fd;
  logInfo(std::string("Playing back ") + deviceLabel(id) + " to " + dev_path);
  return fd;
}

bool InputEventWriter::write(DeviceId id, unsigned short type,
                             unsigned short code, int value) {
  if (id == DeviceId::Keyboard) {
    return keyboard_writer_.write(type, code, value);
  }
  if (id == DeviceId::Mouse) {
    return mouse_writer_.write(type, code, value);
  }
  return writeRaw(id, type, code, value);
}

bool InputEventWriter::writeRaw(DeviceId id, unsigned short type,
                                 unsigned short code, int value) {
  int fd = getFd(id);
  if (fd < 0) return true;  // Skip when device not found
  if (type != EV_SYN) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    Event ev = {tv.tv_sec, tv.tv_usec, type, code, value};
    logDebug(formatEventLine(id, ev, 0));
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
