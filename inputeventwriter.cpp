#include "inputeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/time.h>

#include <cstdio>
#include <sstream>

#include "cursorpos.h"
#include "filesystem.h"
#include "inputdevice.h"
#include "keyboard/keyboardeventwriter.h"
#include "logger.h"
#include "mouse/mouseeventwriter.h"

InputEventWriter::InputEventWriter(FileSystem *fs)
    : fs_(fs), keyboard_writer_(this), mouse_writer_(this, g_cursor) {}

InputEventWriter::~InputEventWriter() {
  for (const auto &p : id_to_fd_) {
    if (p.second >= 0) fs_->close_fd(p.second);
  }
}

int InputEventWriter::get_fd(DeviceId id) {
  if (id == DeviceId::Unknown) return -1;
  auto it = id_to_fd_.find(id);
  if (it != id_to_fd_.end()) return it->second;

  std::string dev_path = find_device_path(id);
  if (dev_path.empty()) {
    log_warn(std::string("No ") + device_label(id) +
             " device found, skipping events.");
    id_to_fd_[id] = -1;
    return -1;
  }

  int fd = fs_->open_read_write(dev_path.c_str());
  if (fd < 0) {
    std::ostringstream oss;
    oss << "Failed to open " << dev_path << " for write (try: sudo)";
    log_warn(oss.str());
    std::perror(dev_path.c_str());
    id_to_fd_[id] = -1;
    return -1;
  }

  id_to_fd_[id] = fd;
  log_info(std::string("Playing back ") + device_label(id) + " to " + dev_path);
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
  return write_raw(id, type, code, value);
}

bool InputEventWriter::write_raw(DeviceId id, unsigned short type,
                                 unsigned short code, int value) {
  int fd = get_fd(id);
  if (fd < 0) return true;  // Skip when device not found
  return write_event_with_sync(fd, type, code, value);
}

bool InputEventWriter::write_event(int fd, unsigned short type,
                                   unsigned short code, int value) {
  struct input_event ev = {};
  gettimeofday(&ev.time, nullptr);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  long n = fs_->write_fd(fd, &ev, sizeof(ev));
  return n == static_cast<long>(sizeof(ev));
}

bool InputEventWriter::write_event_with_sync(int fd, unsigned short type,
                                             unsigned short code, int value) {
  if (!write_event(fd, type, code, value)) {
    std::perror("Failed to write event");
    return false;
  }
  if (type != EV_SYN) {
    bool needs_mt =
        (type == EV_ABS && (code == ABS_MT_POSITION_Y ||
                            (code == ABS_MT_TRACKING_ID && value == -1)));
    if (needs_mt && !write_event(fd, EV_SYN, SYN_MT_REPORT, 0)) {
      std::perror("Failed to write SYN_MT_REPORT");
      return false;
    }
    if (!write_event(fd, EV_SYN, SYN_REPORT, 0)) {
      std::perror("Failed to write SYN_REPORT");
      return false;
    }
  }
  return true;
}
