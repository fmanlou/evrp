#pragma once

// Abstract backend for cursor position. Platform-specific implementations
// (X11, Wayland, etc.) implement this interface.
class CursorPosBackend {
 public:
  virtual ~CursorPosBackend() = default;

  virtual bool getPosition(int *x, int *y) = 0;
  virtual bool isAvailable() = 0;
};
