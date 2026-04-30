#include "evrp/sdk/cursor/cursorpos.h"

#ifdef EVRP_USE_X11_CURSOR
#include "evrp/sdk/cursor/impl/cursorposx11.h"
#endif

ICursorPos *createCursorPos() {
#ifdef EVRP_USE_X11_CURSOR
  ICursorPos *x = createCursorPosBackendX11();
  if (x && x->isAvailable()) {
    return x;
  }
#endif
  return nullptr;
}
