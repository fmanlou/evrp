#include "filesystem/filesystem.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>

FileSystem::FileSystem() : out_(&std::cout) {}

int FileSystem::open_read_only(const char* path, bool nonblocking) const {
  int flags = O_RDONLY;
  if (nonblocking) {
    flags |= O_NONBLOCK;
  }
  return ::open(path, flags);
}

void FileSystem::close_fd(int fd) const {
  if (fd >= 0) {
    ::close(fd);
  }
}

void FileSystem::print_error(const char* message) const { std::perror(message); }

bool FileSystem::open_output(const std::string& path) {
  error_message_.clear();

  if (path.empty()) {
    owned_out_.reset();
    out_ = &std::cout;
    return true;
  }

  std::unique_ptr<std::ofstream> file(new std::ofstream(path));
  if (!file->is_open()) {
    error_message_ = "Failed to open output file: " + path;
    owned_out_.reset();
    out_ = &std::cout;
    return false;
  }

  out_ = file.get();
  owned_out_ = std::move(file);
  return true;
}

std::ostream& FileSystem::output_stream() { return *out_; }

const std::string& FileSystem::error_message() const { return error_message_; }

