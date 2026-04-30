#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "evrp/sdk/filesystem/ifilesystem.h"
#include "evrp/sdk/filesystem/posixfilesystem.h"

class EnhancedFileSystem {
 public:
  EnhancedFileSystem();
  explicit EnhancedFileSystem(IFileSystem *io);

  ~EnhancedFileSystem() = default;

  EnhancedFileSystem(const EnhancedFileSystem &) = delete;
  EnhancedFileSystem &operator=(const EnhancedFileSystem &) = delete;

  int openFd(const std::string &path, int flags, mode_t mode = 0644) const;

  void closeFd(int fd) const;
  long readFd(int fd, void *buffer, unsigned long size) const;
  long writeFd(int fd, const void *buffer, unsigned long size) const;
  int pollFds(int *fds, int nfds, int timeoutMs, bool *ready) const;

  bool writeOutput(int fd, const void *data, size_t size);
  bool writeOutput(int fd, std::string_view data);
  bool flushFd(int fd);

  bool readInputAll(int fd, std::string *out);

 private:
  std::unique_ptr<PosixFileSystem> owned_;
  IFileSystem *io_{nullptr};
};
