#pragma once

#include <map>
#include <memory>

#include "evrp/device/api/types.h"
#include "iraweventwriter.h"
#include "keyboard/keyboardeventwriter.h"
#include "mouse/mouseeventwriter.h"

class FileSystem;

namespace evrp::device::api {
class IPlayback;
}

class RemotePlaybackInjector;

class InputEventWriter : public IRawEventWriter {
 public:
  explicit InputEventWriter(FileSystem *fs);
  ~InputEventWriter();

  bool write(evrp::device::api::DeviceKind device, unsigned short type,
             unsigned short code, int value);

  KeyboardEventWriter *keyboardWriter() { return &keyboardWriter_; }
  MouseEventWriter *mouseWriter() { return &mouseWriter_; }

  /// When non-null, writeRaw sends evdev-shaped events to the device service
  /// (upload + playback per flush) instead of opening local nodes.
  void setRemotePlayback(evrp::device::api::IPlayback *playback);

  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value) override;

 private:
  int getFd(evrp::device::api::DeviceKind device);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                          int value);

  FileSystem *fs_;
  std::map<evrp::device::api::DeviceKind, int> kindToFd_;
  KeyboardEventWriter keyboardWriter_;
  MouseEventWriter mouseWriter_;

  std::unique_ptr<RemotePlaybackInjector> remoteInject_;
};
