#include "mouse/mouseeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "cursor/cursorpos.h"
#include "deviceid.h"
#include "inputeventwriter.h"

MouseEventWriter::MouseEventWriter(InputEventWriter *writer, CursorPos *cursor)
    : writer_(writer), cursor_(cursor) {}

bool MouseEventWriter::move(int dx, int dy) {
  bool ok = true;
  if (dx != 0) ok = writer_->write_raw(DeviceId::Mouse, EV_REL, REL_X, dx);
  if (ok && dy != 0) ok = writer_->write_raw(DeviceId::Mouse, EV_REL, REL_Y, dy);
  return ok;
}

bool MouseEventWriter::move_to_screen(int x, int y) {
  int cur_x = 0, cur_y = 0;
  if (!cursor_ || !cursor_->get_position(&cur_x, &cur_y)) return false;
  return move(x - cur_x, y - cur_y);
}

bool MouseEventWriter::move_to(int x, int y) {
  bool ok = writer_->write_raw(DeviceId::Mouse, EV_ABS, ABS_X, x);
  if (ok) ok = writer_->write_raw(DeviceId::Mouse, EV_ABS, ABS_Y, y);
  return ok;
}

bool MouseEventWriter::move_to_scaled(int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return false;
  const int max_val = 32767;
  int ax = (x < 0) ? 0 : (x > width) ? max_val : (x * max_val / width);
  int ay = (y < 0) ? 0 : (y > height) ? max_val : (y * max_val / height);
  return move_to(ax, ay);
}

bool MouseEventWriter::scroll_v(int value) {
  return writer_->write_raw(DeviceId::Mouse, EV_REL, REL_WHEEL, value);
}

bool MouseEventWriter::scroll_h(int value) {
  return writer_->write_raw(DeviceId::Mouse, EV_REL, REL_HWHEEL, value);
}

bool MouseEventWriter::button_down(unsigned short btn) {
  return writer_->write_raw(DeviceId::Mouse, EV_KEY, btn, 1);
}

bool MouseEventWriter::button_up(unsigned short btn) {
  return writer_->write_raw(DeviceId::Mouse, EV_KEY, btn, 0);
}

bool MouseEventWriter::button_click(unsigned short btn) {
  return button_down(btn) && button_up(btn);
}

bool MouseEventWriter::write(unsigned short type, unsigned short code,
                             int value) {
  return writer_->write_raw(DeviceId::Mouse, type, code, value);
}
