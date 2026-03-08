#pragma once

#include <map>

#include "deviceid.h"
#include "keyboard/keyboardeventwriter.h"
#include "mouse/mouseeventwriter.h"

class FileSystem;

class InputEventWriter {
 public:
  explicit InputEventWriter(FileSystem *fs);
  ~InputEventWriter();

  bool write(DeviceId id, unsigned short type, unsigned short code, int value);

  KeyboardEventWriter *keyboard_writer() { return &keyboard_writer_; }
  MouseEventWriter *mouse_writer() { return &mouse_writer_; }

 private:
  friend class KeyboardEventWriter;
  friend class MouseEventWriter;
  bool write_raw(DeviceId id, unsigned short type, unsigned short code,
                 int value);

  int get_fd(DeviceId id);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  FileSystem *fs_;
  std::map<DeviceId, int> id_to_fd_;
  KeyboardEventWriter keyboard_writer_;
  MouseEventWriter mouse_writer_;
};
