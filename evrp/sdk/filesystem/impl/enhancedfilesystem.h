#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "evrp/sdk/filesystem/enhancedfilesystem.h"

class EnhancedFileSystem final : public IEnhancedFileSystem {
 public:
  EnhancedFileSystem();
  explicit EnhancedFileSystem(IFileSystem *io);

  ~EnhancedFileSystem() override = default;

  EnhancedFileSystem(const EnhancedFileSystem &) = delete;
  EnhancedFileSystem &operator=(const EnhancedFileSystem &) = delete;

  int openFd(const std::string &path, int flags, mode_t mode) const override;

  void closeFd(int fd) const override;
  long readFd(int fd, void *buffer, unsigned long size) const override;
  long writeFd(int fd, const void *buffer, unsigned long size) const override;
  int pollFds(int *fds, int nfds, int timeoutMs, bool *ready) const override;

  bool writeOutput(int fd, const void *data, size_t size) override;
  bool writeOutput(int fd, std::string_view data) override;
  bool flushFd(int fd) override;

  bool readInputAll(int fd, std::string *out) override;

 private:
  std::unique_ptr<IFileSystem> owned_;
  IFileSystem *io_{nullptr};
};
