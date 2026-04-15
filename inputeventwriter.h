#pragma once

#include <map>

#include "evrp/device/api/types.h"
#include "keyboard/keyboardeventwriter.h"
#include "mouse/mouseeventwriter.h"

class FileSystem;

class InputEventWriter {
 public:
  explicit InputEventWriter(FileSystem *fs);
  ~InputEventWriter();

  bool write(evrp::device::api::DeviceKind device, unsigned short type,
             unsigned short code, int value);

  KeyboardEventWriter *keyboardWriter() { return &keyboardWriter_; }
  MouseEventWriter *mouseWriter() { return &mouseWriter_; }

 private:
  friend class KeyboardEventWriter;
  friend class MouseEventWriter;
  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value);

  int getFd(evrp::device::api::DeviceKind device);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                          int value);

  FileSystem *fs_;
  std::map<evrp::device::api::DeviceKind, int> kindToFd_;
  KeyboardEventWriter keyboardWriter_;
  MouseEventWriter mouseWriter_;
};
