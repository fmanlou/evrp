#pragma once

#include <fstream>
#include <istream>
#include <memory>
#include <ostream>
#include <string>

class FileSystem {
 public:
  FileSystem();

  int openReadOnly(const char *path, bool nonblocking) const;
  int openReadWrite(const char *path) const;
  void closeFd(int fd) const;
  long readFd(int fd, void *buffer, unsigned long size) const;
  long writeFd(int fd, const void *buffer, unsigned long size) const;
  int pollFds(int *fds, int nfds, int timeoutMs, bool *ready) const;

  // Empty path means writing to stdout.
  bool openOutput(const std::string &path);
  std::ostream &outputStream();
  const std::string &errorMessage() const;

  bool openInput(const std::string &path);
  std::istream &inputStream();

 private:
  std::unique_ptr<std::ostream> ownedOut_;
  std::ostream *out_;
  std::unique_ptr<std::ifstream> ownedIn_;
  std::istream *in_;
  std::string errorMessage_;
};
