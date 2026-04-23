#pragma once

class InputEventWriter;

class KeyboardEventWriter {
 public:
  explicit KeyboardEventWriter(InputEventWriter *writer);

  bool press(unsigned short key_code);
  bool release(unsigned short key_code);
  bool repeat(unsigned short key_code);

  
  bool write(unsigned short type, unsigned short code, int value);

 private:
  InputEventWriter *writer_;
};
