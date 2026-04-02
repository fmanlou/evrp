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

  KeyboardEventWriter *keyboardWriter() { return &keyboard_writer_; }
  MouseEventWriter *mouseWriter() { return &mouse_writer_; }

 private:
  friend class KeyboardEventWriter;
  friend class MouseEventWriter;
  bool writeRaw(DeviceId id, unsigned short type, unsigned short code,
                 int value);

  int getFd(DeviceId id);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                             int value);

  FileSystem *fs_;
  std::map<DeviceId, int> id_to_fd_;
  KeyboardEventWriter keyboard_writer_;
  MouseEventWriter mouse_writer_;
};
