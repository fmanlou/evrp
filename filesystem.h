#pragma once

#include <fstream>
#include <istream>
#include <memory>
#include <ostream>
#include <string>

class FileSystem {
 public:
  FileSystem();

  int open_read_only(const char* path, bool nonblocking) const;
  int open_read_write(const char* path) const;
  void close_fd(int fd) const;
  long read_fd(int fd, void* buffer, unsigned long size) const;
  long write_fd(int fd, const void* buffer, unsigned long size) const;
  int poll_fds(int* fds, int nfds, int timeout_ms, bool* ready) const;

  // Empty path means writing to stdout.
  bool open_output(const std::string& path);
  std::ostream& output_stream();
  const std::string& error_message() const;

  bool open_input(const std::string& path);
  std::istream& input_stream();

 private:
  std::unique_ptr<std::ostream> owned_out_;
  std::ostream* out_;
  std::unique_ptr<std::ifstream> owned_in_;
  std::istream* in_;
  std::string error_message_;
};
