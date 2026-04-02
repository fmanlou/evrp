#pragma once

#include "cursorposbackend.h"

// X11 backend for cursor position. Uses XQueryPointer.
CursorPosBackend *createCursorPosBackendX11();
