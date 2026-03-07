#pragma once

#include <map>
#include <string>

class FileSystem;

class InputEventWriter {
 public:
  explicit InputEventWriter(FileSystem* fs);
  ~InputEventWriter();

  bool write_keyboard(unsigned short type, unsigned short code, int value);
  bool write_mouse(unsigned short type, unsigned short code, int value);
  bool write_touchpad(unsigned short type, unsigned short code, int value);

 private:
  int get_fd(const std::string& label);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  FileSystem* fs_;
  std::map<std::string, int> label_to_fd_;
};
