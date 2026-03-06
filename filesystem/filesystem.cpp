#include "filesystem/filesystem.h"

#include <fcntl.h>
#include <poll.h>
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

