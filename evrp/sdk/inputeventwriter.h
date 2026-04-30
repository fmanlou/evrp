#pragma once

#include <map>

#include "evrp/device/api/types.h"
#include "evrp/sdk/iraweventwriter.h"
#include "evrp/sdk/keyboard/keyboardeventwriter.h"
#include "evrp/sdk/mouse/mouseeventwriter.h"

class IEnhancedFileSystem;

class InputEventWriter : public IRawEventWriter {
 public:
  explicit InputEventWriter(IEnhancedFileSystem *fs);
  ~InputEventWriter();

  bool write(evrp::device::api::DeviceKind device, unsigned short type,
             unsigned short code, int value);

  KeyboardEventWriter *keyboardWriter() { return &keyboardWriter_; }
  MouseEventWriter *mouseWriter() { return &mouseWriter_; }

  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value) override;

 private:
  int getFd(evrp::device::api::DeviceKind device);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                          int value);

  IEnhancedFileSystem *fs_;
  std::map<evrp::device::api::DeviceKind, int> kindToFd_;
  KeyboardEventWriter keyboardWriter_;
  MouseEventWriter mouseWriter_;
};
