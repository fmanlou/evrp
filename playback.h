#pragma once

#include "argparser.h"
#include "asynclogwriter.h"
#include "filesystem/filesystem.h"

#include <map>
#include <string>

class Playback {
 public:
  explicit Playback(const run_options& options);
  int run();

 private:
  int get_fd(const std::string& label);
  bool write_event(int fd, unsigned short type, unsigned short code, int value);
  bool write_event_with_sync(int fd, unsigned short type, unsigned short code,
                             int value);

  run_options options_;
  FileSystem fs_;
  std::map<std::string, int> label_to_fd_;
  AsyncLogWriter log_writer_;
};
