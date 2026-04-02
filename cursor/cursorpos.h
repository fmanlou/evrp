#pragma once

class CursorPosBackend;

class CursorPos {
 public:
  CursorPos() = default;

  bool getPosition(int *x, int *y);

  bool isAvailable() const;

 private:
  void ensureInitialized() const;

  mutable bool initialized_ = false;
  mutable bool available_ = false;
  mutable CursorPosBackend *backend_ = nullptr;
};

extern CursorPos *g_cursor;

inline bool getCursorPosition(int *x, int *y) {
  return g_cursor && g_cursor->getPosition(x, y);
}
