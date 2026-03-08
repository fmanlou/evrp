#pragma once

class InputEventWriter;

// Mouse event writer: movement, scroll, button press/release.
// EV_REL: REL_X, REL_Y (move), REL_WHEEL, REL_HWHEEL (scroll).
// EV_KEY: BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, etc.
class MouseEventWriter {
 public:
  explicit MouseEventWriter(InputEventWriter *writer);

  bool move(int dx, int dy);
  bool scroll_v(int value);  // Vertical: positive=down, negative=up
  bool scroll_h(int value);  // Horizontal: positive=right, negative=left

  bool button_down(unsigned short btn);
  bool button_up(unsigned short btn);
  bool button_click(unsigned short btn);

  // Writes any mouse event (for playback passthrough).
  bool write(unsigned short type, unsigned short code, int value);

 private:
  InputEventWriter *writer_;
};
