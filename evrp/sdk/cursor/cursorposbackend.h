#pragma once

class CursorPosBackend {
 public:
  virtual ~CursorPosBackend() = default;

  virtual bool getPosition(int *x, int *y) = 0;
  virtual bool isAvailable() = 0;
};
