#pragma once

#include <map>

#include "evrp/sdk/types.h"
#include "evrp/sdk/iraweventwriter.h"
#include "evrp/sdk/keyboard/keyboardeventwriter.h"
#include "evrp/sdk/mouse/mouseeventwriter.h"

class IEnhancedFileSystem;
class ICursorPos;

class InputEventWriter : public IRawEventWriter {
 public:
  InputEventWriter(IEnhancedFileSystem *fs, ICursorPos *cursor = nullptr);
  ~InputEventWriter();

  bool write(evrp::sdk::DeviceKind device, unsigned short type,
             unsigned short code, int value);

  KeyboardEventWriter *keyboardWriter() { return &keyboardWriter_; }
  MouseEventWriter *mouseWriter() { return &mouseWriter_; }

  bool writeRaw(evrp::sdk::DeviceKind device, unsigned short type,
                unsigned short code, int value) override;

 private:
  int getFd(evrp::sdk::DeviceKind device);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                          int value);

  IEnhancedFileSystem *fs_;
  std::map<evrp::sdk::DeviceKind, int> kindToFd_;
  KeyboardEventWriter keyboardWriter_;
  MouseEventWriter mouseWriter_;
};
