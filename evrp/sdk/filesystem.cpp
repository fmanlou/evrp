#include "evrp/sdk/filesystem.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

int FileSystem::openReadOnly(const char *path, bool nonblocking) const {
  int flags = O_RDONLY;
  if (nonblocking) {
    flags |= O_NONBLOCK;
  }
  return ::open(path, flags);
}

int FileSystem::openReadWrite(const char *path) const {
  return ::open(path, O_RDWR);
}

void FileSystem::closeFd(int fd) const {
  if (fd >= 0) {
    ::close(fd);
  }
}

long FileSystem::readFd(int fd, void *buffer, unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) return -1;
  return static_cast<long>(::read(fd, buffer, size));
}

long FileSystem::writeFd(int fd, const void *buffer,
                          unsigned long size) const {
  if (fd < 0 || !buffer || size == 0) return -1;
  return static_cast<long>(::write(fd, buffer, size));
}

int FileSystem::pollFds(int *fds, int nfds, int timeoutMs,
                         bool *ready) const {
  if (!fds || nfds <= 0 || !ready) return -1;

  struct pollfd pfds[32];
  int n = (nfds > 32) ? 32 : nfds;
  for (int i = 0; i < n; ++i) {
    pfds[i].fd = fds[i];
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
    ready[i] = false;
  }

  int ret = ::poll(pfds, static_cast<nfds_t>(n), timeoutMs);
  if (ret < 0) return -1;
  if (ret == 0) return 0;

  int count = 0;
  for (int i = 0; i < n; ++i) {
    ready[i] = (pfds[i].revents & POLLIN) != 0;
    if (ready[i]) ++count;
  }
  return count;
}

int FileSystem::openOutput(const std::string &path) {
  errorMessage_.clear();
  if (path.empty()) {
    return STDOUT_FILENO;
  }
  int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    errorMessage_ = "Failed to open output file: " + path;
    return -1;
  }
  return fd;
}

bool FileSystem::writeOutput(int fd, const void *data, size_t size) {
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
    ssize_t n = ::write(fd, p, remaining);
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

bool FileSystem::writeOutput(int fd, std::string_view data) {
  return writeOutput(
      fd, data.empty() ? static_cast<const void *>(nullptr) : data.data(),
      data.size());
}

bool FileSystem::flushOutput(int fd) {
  if (fd < 0) {
    return false;
  }
  if (fd == STDOUT_FILENO) {
    return std::fflush(stdout) == 0;
  }
  return ::fsync(fd) == 0;
}

const std::string &FileSystem::errorMessage() const { return errorMessage_; }

int FileSystem::openInput(const std::string &path) {
  errorMessage_.clear();
  int fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    errorMessage_ = "Failed to open input file: " + path;
    return -1;
  }
  return fd;
}

bool FileSystem::readInputAll(int fd, std::string *out) {
  if (!out || fd < 0) {
    return false;
  }
  out->clear();
  char buf[8192];
  for (;;) {
    ssize_t n = ::read(fd, buf, sizeof buf);
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
