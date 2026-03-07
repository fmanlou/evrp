#include "filesystem/filesystem.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>

FileSystem::FileSystem() : out_(&std::cout), in_(nullptr) {}

int FileSystem::open_read_only(const char* path, bool nonblocking) const {
  int flags = O_RDONLY;
  if (nonblocking) {
    flags |= O_NONBLOCK;
  }
  return ::open(path, flags);
}

int FileSystem::open_read_write(const char* path) const {
  return ::open(path, O_RDWR);
}

void FileSystem::close_fd(int fd) const {
  if (fd >= 0) {
    ::close(fd);
  }
}

long FileSystem::read_fd(int fd, void* buffer, unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) return -1;
  return static_cast<long>(::read(fd, buffer, size));
}

long FileSystem::write_fd(int fd, const void* buffer, unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) return -1;
  return static_cast<long>(::write(fd, buffer, size));
}

int FileSystem::poll_fds(int* fds, int nfds, int timeout_ms, bool* ready) const {
  if (!fds || nfds <= 0 || !ready) return -1;

  struct pollfd pfds[32];
  int n = (nfds > 32) ? 32 : nfds;
  for (int i = 0; i < n; ++i) {
    pfds[i].fd = fds[i];
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
    ready[i] = false;
  }

  int ret = ::poll(pfds, static_cast<nfds_t>(n), timeout_ms);
  if (ret < 0) return -1;
  if (ret == 0) return 0;

  int count = 0;
  for (int i = 0; i < n; ++i) {
    ready[i] = (pfds[i].revents & POLLIN) != 0;
    if (ready[i]) ++count;
  }
  return count;
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

bool FileSystem::open_input(const std::string& path) {
  error_message_.clear();
  std::unique_ptr<std::ifstream> file(new std::ifstream(path));
  if (!file->is_open()) {
    error_message_ = "Failed to open input file: " + path;
    owned_in_.reset();
    in_ = nullptr;
    return false;
  }
  in_ = file.get();
  owned_in_ = std::move(file);
  return true;
}

std::istream& FileSystem::input_stream() { return *in_; }

