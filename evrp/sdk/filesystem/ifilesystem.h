#pragma once

#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstddef>

class IFileSystem {
 public:
  virtual ~IFileSystem() = default;

  IFileSystem() = default;
  IFileSystem(const IFileSystem &) = delete;
  IFileSystem &operator=(const IFileSystem &) = delete;

  virtual int open(const char *path, int flags) const = 0;
  virtual int open(const char *path, int flags, mode_t mode) const = 0;
  virtual int close(int fd) const = 0;
  virtual ssize_t read(int fd, void *buf, size_t count) const = 0;
  virtual ssize_t write(int fd, const void *buf, size_t count) const = 0;
  virtual int poll(struct pollfd *fds, nfds_t nfds, int timeoutMs) const = 0;
  virtual int fsync(int fd) const = 0;
  virtual int fflushStdout() const = 0;
};
