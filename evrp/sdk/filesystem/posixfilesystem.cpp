#include "evrp/sdk/filesystem/posixfilesystem.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>

int PosixFileSystem::open(const char *path, int flags) const {
  return ::open(path, flags);
}

int PosixFileSystem::open(const char *path, int flags, mode_t mode) const {
  return ::open(path, flags, mode);
}

int PosixFileSystem::close(int fd) const {
  return ::close(fd);
}

ssize_t PosixFileSystem::read(int fd, void *buf, size_t count) const {
  return ::read(fd, buf, count);
}

ssize_t PosixFileSystem::write(int fd, const void *buf, size_t count) const {
  return ::write(fd, buf, count);
}

int PosixFileSystem::poll(struct pollfd *fds, nfds_t nfds,
                         int timeoutMs) const {
  return ::poll(fds, nfds, timeoutMs);
}

int PosixFileSystem::fsync(int fd) const {
  return ::fsync(fd);
}

int PosixFileSystem::fflushStdout() const {
  return std::fflush(stdout);
}
