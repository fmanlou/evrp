#pragma once

class InputEventWriter;

// Converts raw input device events to press/release interface.
// For EV_KEY: value 0 -> release, 1 -> press, 2 -> repeat.
class KeyboardEventWriter {
 public:
  explicit KeyboardEventWriter(InputEventWriter *writer);

  bool press(unsigned short key_code);
  bool release(unsigned short key_code);
  bool repeat(unsigned short key_code);

  // Writes any keyboard event. EV_KEY -> press/release/repeat; others -> raw.
  bool write(unsigned short type, unsigned short code, int value);

 private:
  InputEventWriter *writer_;
};
