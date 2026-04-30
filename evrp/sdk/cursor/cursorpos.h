#pragma once

class ICursorPos {
 public:
  virtual ~ICursorPos() = default;

  virtual bool getPosition(int *x, int *y) = 0;
  virtual bool isAvailable() = 0;
};

ICursorPos *createCursorPos();
