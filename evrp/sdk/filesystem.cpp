#include "evrp/sdk/filesystem.h"

#include <fcntl.h>

#include <cerrno>
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

EnhancedFileSystem::EnhancedFileSystem()
    : owned_(std::make_unique<PosixFileSystem>()), io_(owned_.get()) {}

EnhancedFileSystem::EnhancedFileSystem(IFileSystem *io) : io_(io) {}

int EnhancedFileSystem::openFd(const std::string &path, int flags,
                               mode_t mode) const {
  if (path.empty()) {
    const int acc = flags & O_ACCMODE;
    if (acc == O_WRONLY || acc == O_RDWR) {
      return STDOUT_FILENO;
    }
    errno = EINVAL;
    return -1;
  }
  if (flags & O_CREAT) {
    return io_->open(path.c_str(), flags, mode);
  }
  return io_->open(path.c_str(), flags);
}

void EnhancedFileSystem::closeFd(int fd) const {
  if (fd >= 0) {
    io_->close(fd);
  }
}

long EnhancedFileSystem::readFd(int fd, void *buffer,
                                unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) {
    return -1;
  }
  return static_cast<long>(
      io_->read(fd, buffer, static_cast<size_t>(size)));
}

long EnhancedFileSystem::writeFd(int fd, const void *buffer,
                                  unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) {
    return -1;
  }
  return static_cast<long>(
      io_->write(fd, buffer, static_cast<size_t>(size)));
}

int EnhancedFileSystem::pollFds(int *fds, int nfds, int timeoutMs,
                                 bool *ready) const {
  if (!fds || nfds <= 0 || !ready) {
    return -1;
  }

  struct pollfd pfds[32];
  int n = (nfds > 32) ? 32 : nfds;
  for (int i = 0; i < n; ++i) {
    pfds[i].fd = fds[i];
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
    ready[i] = false;
  }

  int ret = io_->poll(pfds, static_cast<nfds_t>(n), timeoutMs);
  if (ret < 0) {
    return -1;
  }
  if (ret == 0) {
    return 0;
  }

  int count = 0;
  for (int i = 0; i < n; ++i) {
    ready[i] = (pfds[i].revents & POLLIN) != 0;
    if (ready[i]) {
      ++count;
    }
  }
  return count;
}

bool EnhancedFileSystem::writeOutput(int fd, const void *data, size_t size) {
  if (fd < 0) {
    return false;
  }
  if (size == 0) {
    return true;
  }
  if (!data) {
    return false;
  }
  const char *p = static_cast<const char *>(data);
  size_t remaining = size;
  while (remaining > 0) {
    ssize_t n = io_->write(fd, p, remaining);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    if (n == 0) {
      return false;
    }
    p += n;
    remaining -= static_cast<size_t>(n);
  }
  return true;
}

bool EnhancedFileSystem::writeOutput(int fd, std::string_view data) {
  return writeOutput(
      fd, data.empty() ? static_cast<const void *>(nullptr) : data.data(),
      data.size());
}

bool EnhancedFileSystem::flushFd(int fd) {
  if (fd < 0) {
    return false;
  }
  if (fd == STDOUT_FILENO) {
    return io_->fflushStdout() == 0;
  }
  return io_->fsync(fd) == 0;
}

bool EnhancedFileSystem::readInputAll(int fd, std::string *out) {
  if (!out || fd < 0) {
    return false;
  }
  out->clear();
  char buf[8192];
  for (;;) {
    ssize_t n = io_->read(fd, buf, sizeof buf);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    if (n == 0) {
      break;
    }
    out->append(buf, static_cast<size_t>(n));
  }
  return true;
}
