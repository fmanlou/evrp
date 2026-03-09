#pragma once

#include "cursorposbackend.h"

// X11 backend for cursor position. Uses XQueryPointer.
CursorPosBackend *create_cursor_pos_backend_x11();
