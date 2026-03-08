#include "cursorpos.h"

#include "cursorposbackend.h"
#ifdef EVRP_USE_X11_CURSOR
#include "cursorposx11.h"
#endif

CursorPos *g_cursor = nullptr;

void CursorPos::ensure_initialized() const {
  if (initialized_) return;

  // Try backends in order. Add Wayland, etc. here.
  CursorPosBackend *b = nullptr;
#ifdef EVRP_USE_X11_CURSOR
  b = create_cursor_pos_backend_x11();
#endif
  if (b && b->is_available()) {
    backend_ = b;
    available_ = true;
  }
  initialized_ = true;
}

bool CursorPos::is_available() const {
  ensure_initialized();
  return available_;
}

bool CursorPos::get_position(int *x, int *y) {
  if (!x || !y) return false;
  ensure_initialized();
  if (!available_ || !backend_) return false;
  return backend_->get_position(x, y);
}
