#pragma once

#include <string>
#include <string_view>

class FileSystem {
 public:
  FileSystem() = default;
  ~FileSystem() = default;

  FileSystem(const FileSystem &) = delete;
  FileSystem &operator=(const FileSystem &) = delete;

  int openReadOnly(const char *path, bool nonblocking) const;
  int openReadWrite(const char *path) const;
  void closeFd(int fd) const;
  long readFd(int fd, void *buffer, unsigned long size) const;
  long writeFd(int fd, const void *buffer, unsigned long size) const;
  int pollFds(int *fds, int nfds, int timeoutMs, bool *ready) const;

  /// Empty \a path: returns `STDOUT_FILENO` (do not `closeFd`). Non-empty: opened
  /// fd on success; caller must `closeFd` when done. Returns -1 on failure.
  int openOutput(const std::string &path);
  bool writeOutput(int fd, const void *data, size_t size);
  bool writeOutput(int fd, std::string_view data);
  bool flushOutput(int fd);

  const std::string &errorMessage() const;

  /// Returns read fd or -1. Caller must `closeFd` when done.
  int openInput(const std::string &path);
  bool readInputAll(int fd, std::string *out);

 private:
  std::string errorMessage_;
};
