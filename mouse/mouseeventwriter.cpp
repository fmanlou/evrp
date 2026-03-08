#include "mouse/mouseeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "deviceid.h"
#include "inputeventwriter.h"

MouseEventWriter::MouseEventWriter(InputEventWriter *writer) : writer_(writer) {}

bool MouseEventWriter::move(int dx, int dy) {
  bool ok = true;
  if (dx != 0) ok = writer_->write_raw(DeviceId::Mouse, EV_REL, REL_X, dx);
  if (ok && dy != 0) ok = writer_->write_raw(DeviceId::Mouse, EV_REL, REL_Y, dy);
  return ok;
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
