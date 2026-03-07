#pragma once

#include "asynclogwriter.h"
#include "filesystem/filesystem.h"

#include <map>
#include <string>

class Playback {
 public:
  Playback(const std::string& path, bool quiet);
  int run();

 private:
  int get_fd(const std::string& label);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  std::string path_;
  bool quiet_;
  FileSystem fs_;
  std::map<std::string, int> label_to_fd_;
  AsyncLogWriter log_writer_;
};
