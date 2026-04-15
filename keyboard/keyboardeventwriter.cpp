#include "keyboard/keyboardeventwriter.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>

#include "evrp/device/api/types.h"
#include "inputeventwriter.h"

KeyboardEventWriter::KeyboardEventWriter(InputEventWriter *writer)
    : writer_(writer) {}

bool KeyboardEventWriter::press(unsigned short key_code) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kKeyboard, EV_KEY,
                           key_code, 1);
}

bool KeyboardEventWriter::release(unsigned short key_code) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kKeyboard, EV_KEY,
                           key_code, 0);
}

bool KeyboardEventWriter::repeat(unsigned short key_code) {
  return writer_->writeRaw(evrp::device::api::DeviceKind::kKeyboard, EV_KEY,
                           key_code, 2);
}

bool KeyboardEventWriter::write(unsigned short type, unsigned short code,
                                int value) {
  if (type == EV_KEY) {
    if (value == 0) return release(code);
    if (value == 1) return press(code);
    if (value == 2) return repeat(code);
    return true;  // Unknown value, skip
  }
  return writer_->writeRaw(evrp::device::api::DeviceKind::kKeyboard, type,
                           code, value);
}
