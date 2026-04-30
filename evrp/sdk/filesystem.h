#pragma once

#include <memory>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <string_view>

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
