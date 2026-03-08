#pragma once

#include <map>

#include "deviceid.h"
#include "keyboardeventwriter.h"

class FileSystem;

class InputEventWriter {
 public:
  explicit InputEventWriter(FileSystem *fs);
  ~InputEventWriter();

  bool write(DeviceId id, unsigned short type, unsigned short code, int value);

 private:
  friend class KeyboardEventWriter;
  bool write_raw(DeviceId id, unsigned short type, unsigned short code,
                 int value);

  int get_fd(DeviceId id);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  FileSystem *fs_;
  std::map<DeviceId, int> id_to_fd_;
  KeyboardEventWriter keyboard_writer_;
};
