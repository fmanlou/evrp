#pragma once

#include "evrp/sdk/filesystem/filesystem.h"

class PosixFileSystem final : public IFileSystem {
 public:
  int open(const char *path, int flags) const override;
  int open(const char *path, int flags, mode_t mode) const override;
  int close(int fd) const override;
  ssize_t read(int fd, void *buf, size_t count) const override;
  ssize_t write(int fd, const void *buf, size_t count) const override;
  int poll(struct pollfd *fds, nfds_t nfds, int timeoutMs) const override;
  int fsync(int fd) const override;
  int fflushStdout() const override;
};
