#pragma once

#include <map>

#include "evrp/device/api/types.h"
#include "keyboard/keyboardeventwriter.h"
#include "mouse/mouseeventwriter.h"

#include <chrono>
#include <cstdint>
#include <vector>

class FileSystem;

namespace evrp::device::api {
class IPlayback;
}

class InputEventWriter {
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

 private:
  friend class KeyboardEventWriter;
  friend class MouseEventWriter;
  bool writeRaw(evrp::device::api::DeviceKind device, unsigned short type,
                unsigned short code, int value);
  bool writeRawRemote(evrp::device::api::DeviceKind device, unsigned short type,
                      unsigned short code, int value);
  bool flushRemoteBatch(std::vector<evrp::device::api::InputEvent> batch);

  int getFd(evrp::device::api::DeviceKind device);
  bool writeEvent(int fd, unsigned short type, unsigned short code, int value);
  bool writeEventWithSync(int fd, unsigned short type, unsigned short code,
                          int value);

  FileSystem *fs_;
  std::map<evrp::device::api::DeviceKind, int> kindToFd_;
  KeyboardEventWriter keyboardWriter_;
  MouseEventWriter mouseWriter_;

  evrp::device::api::IPlayback *remotePlayback_ = nullptr;
  int64_t remoteTimelineUs_ = 0;
  bool remoteHasWall_ = false;
  std::chrono::steady_clock::time_point remoteLastWall_{};
};
