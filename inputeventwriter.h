#pragma once

#include <map>

#include "deviceid.h"

class FileSystem;

class InputEventWriter {
 public:
  explicit InputEventWriter(FileSystem *fs);
  ~InputEventWriter();

  bool write(DeviceId id, unsigned short type, unsigned short code, int value);

 private:
  int get_fd(DeviceId id);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  FileSystem *fs_;
  std::map<DeviceId, int> id_to_fd_;
};
