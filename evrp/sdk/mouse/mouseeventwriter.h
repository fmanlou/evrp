#pragma once

class ICursorPos;
class IRawEventWriter;

class MouseEventWriter {
 public:
  MouseEventWriter(IRawEventWriter *writer, ICursorPos *cursor);

  bool move(int dx, int dy);


  bool moveToScreen(int x, int y);

  bool moveTo(int x, int y);

  bool moveToScaled(int x, int y, int width, int height);
  bool scrollV(int value);
  bool scrollH(int value);

  bool buttonDown(unsigned short btn);
  bool buttonUp(unsigned short btn);
  bool buttonClick(unsigned short btn);


  bool write(unsigned short type, unsigned short code, int value);

  ICursorPos *cursorPos() const { return cursor_; }

 private:
  IRawEventWriter *writer_;
  ICursorPos *cursor_;
};
