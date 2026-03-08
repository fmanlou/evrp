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

  // Dispatches raw event: if EV_KEY, converts to press/release/repeat.
  // Returns true if event was handled (keyboard) and written successfully.
  bool dispatch(unsigned short type, unsigned short code, int value);

 private:
  InputEventWriter *writer_;
};
