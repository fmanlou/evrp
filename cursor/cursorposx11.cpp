#include "cursorposx11.h"

#include <X11/Xlib.h>

namespace {

class CursorPosBackendX11 : public CursorPosBackend {
 public:
  bool getPosition(int *x, int *y) override {
    if (!x || !y) return false;

    Display *d = XOpenDisplay(nullptr);
    if (!d) return false;

    Window root = DefaultRootWindow(d);
    Window child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    Bool ok = XQueryPointer(d, root, &root, &child, &root_x, &root_y,
                             &win_x, &win_y, &mask);
    XCloseDisplay(d);

    if (!ok) return false;
    *x = root_x;
    *y = root_y;
    return true;
  }

  bool isAvailable() override {
    Display *d = XOpenDisplay(nullptr);
    if (!d) return false;
    XCloseDisplay(d);
    return true;
  }
};

}  

CursorPosBackend *createCursorPosBackendX11() {
  static CursorPosBackendX11 instance;
  return &instance;
}
