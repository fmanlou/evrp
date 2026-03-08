#pragma once

class CursorPosBackend;

// Cursor position via display server backend (X11 when available).
// Detects X11 support at runtime; get_position returns false if unavailable.
class CursorPos {
 public:
  CursorPos() = default;

  // Get current cursor position. Returns true on success.
  bool get_position(int *x, int *y);

  // Whether a backend (X11) is available.
  bool is_available() const;

 private:
  void ensure_initialized() const;

  mutable bool initialized_ = false;
  mutable bool available_ = false;
  mutable CursorPosBackend *backend_ = nullptr;
};

extern CursorPos *g_cursor;

inline bool get_cursor_position(int *x, int *y) {
  return g_cursor && g_cursor->get_position(x, y);
}
