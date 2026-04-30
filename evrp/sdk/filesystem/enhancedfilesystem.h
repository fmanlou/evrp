#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "evrp/sdk/filesystem/filesystem.h"

class IEnhancedFileSystem {
 public:
  virtual ~IEnhancedFileSystem() = default;

  IEnhancedFileSystem() = default;
  IEnhancedFileSystem(const IEnhancedFileSystem &) = delete;
  IEnhancedFileSystem &operator=(const IEnhancedFileSystem &) = delete;

  virtual int openFd(const std::string &path, int flags,
                     mode_t mode) const = 0;
  virtual void closeFd(int fd) const = 0;
  virtual long readFd(int fd, void *buffer, unsigned long size) const = 0;
  virtual long writeFd(int fd, const void *buffer,
                       unsigned long size) const = 0;
  virtual int pollFds(int *fds, int nfds, int timeoutMs,
                      bool *ready) const = 0;
  virtual bool writeOutput(int fd, const void *data, size_t size) = 0;
  virtual bool writeOutput(int fd, std::string_view data) = 0;
  virtual bool flushFd(int fd) = 0;
  virtual bool readInputAll(int fd, std::string *out) = 0;
};

std::unique_ptr<IEnhancedFileSystem> createEnhancedFileSystem();

std::unique_ptr<IEnhancedFileSystem> createEnhancedFileSystem(IFileSystem *io);
