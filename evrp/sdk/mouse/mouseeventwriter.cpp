#include "evrp/sdk/mouse/mouseeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "cursor/cursorpos.h"
#include "evrp/device/api/types.h"
#include "evrp/sdk/iraweventwriter.h"

MouseEventWriter::MouseEventWriter(IRawEventWriter *writer, CursorPos *cursor)
    : writer_(writer), cursor_(cursor) {}

bool MouseEventWriter::move(int dx, int dy) {
  bool ok = true;
  if (dx != 0) ok = writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_REL, REL_X, dx);
  if (ok && dy != 0) ok = writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_REL, REL_Y, dy);
  return ok;
}

bool MouseEventWriter::moveToScreen(int x, int y) {
  int cur_x = 0, cur_y = 0;
  if (!cursor_ || !cursor_->getPosition(&cur_x, &cur_y)) return false;
  return move(x - cur_x, y - cur_y);
}

bool MouseEventWriter::moveTo(int x, int y) {
  bool ok = writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_ABS, ABS_X, x);
  if (ok) ok = writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_ABS, ABS_Y, y);
  return ok;
}

bool MouseEventWriter::moveToScaled(int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return false;
  const int max_val = 32767;
  int ax = (x < 0) ? 0 : (x > width) ? max_val : (x * max_val / width);
  int ay = (y < 0) ? 0 : (y > height) ? max_val : (y * max_val / height);
  return moveTo(ax, ay);
}

bool MouseEventWriter::scrollV(int value) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_REL, REL_WHEEL, value);
}

bool MouseEventWriter::scrollH(int value) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_REL, REL_HWHEEL, value);
}

bool MouseEventWriter::buttonDown(unsigned short btn) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_KEY, btn, 1);
}

bool MouseEventWriter::buttonUp(unsigned short btn) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, EV_KEY, btn, 0);
}

bool MouseEventWriter::buttonClick(unsigned short btn) {
  return buttonDown(btn) && buttonUp(btn);
}

bool MouseEventWriter::write(unsigned short type, unsigned short code,
                             int value) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kMouse, type, code, value);
}
