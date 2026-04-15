#include "cursorpos.h"

#include "cursorposbackend.h"
#ifdef EVRP_USE_X11_CURSOR
#include "cursorposx11.h"
#endif

CursorPos *gCursor = nullptr;

void CursorPos::ensureInitialized() const {
  if (initialized_) return;

  // Try backends in order. Add Wayland, etc. here.
  CursorPosBackend *b = nullptr;
#ifdef EVRP_USE_X11_CURSOR
  b = createCursorPosBackendX11();
#endif
  if (b && b->isAvailable()) {
    backend_ = b;
    available_ = true;
  }
  initialized_ = true;
}

bool CursorPos::isAvailable() const {
  ensureInitialized();
  return available_;
}

bool CursorPos::getPosition(int *x, int *y) {
  if (!x || !y) return false;
  ensureInitialized();
  if (!available_ || !backend_) return false;
  return backend_->getPosition(x, y);
}
