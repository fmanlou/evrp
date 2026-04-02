#pragma once

class CursorPos;
class InputEventWriter;

// Mouse event writer: movement, scroll, button press/release.
// EV_REL: REL_X, REL_Y (move), REL_WHEEL, REL_HWHEEL (scroll).
// EV_KEY: BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, etc.
class MouseEventWriter {
 public:
  MouseEventWriter(InputEventWriter *writer, CursorPos *cursor);

  bool move(int dx, int dy);
  // Get current position via X11, then move by delta to reach (x, y).
  // Works with REL-only mice. Requires DISPLAY (X11/XWayland).
  bool moveToScreen(int x, int y);
  // Absolute position. Coordinates typically 0-32767 (device range).
  bool moveTo(int x, int y);
  // Scale from (0,0)-(width,height) to device range (0-32767).
  bool moveToScaled(int x, int y, int width, int height);
  bool scrollV(int value);  // Vertical: positive=down, negative=up
  bool scrollH(int value);  // Horizontal: positive=right, negative=left

  bool buttonDown(unsigned short btn);
  bool buttonUp(unsigned short btn);
  bool buttonClick(unsigned short btn);

  // Writes any mouse event (for playback passthrough).
  bool write(unsigned short type, unsigned short code, int value);

 private:
  InputEventWriter *writer_;
  CursorPos *cursor_;
};
