#pragma once

#include <atomic>
#include <string>

#include "log/common/log.h"

namespace evrp::sdk {

/** Consumes log lines (e.g. from a remote peer). */
class ILogReceiveService {
 public:
  virtual ~ILogReceiveService() = default;

  virtual logging::LogLevel getLevel() const = 0;

  virtual void setLevel(logging::LogLevel level) = 0;

  virtual void log(logging::LogLevel level, std::string&& msg) = 0;

  virtual void flush() = 0;
};

/** Produces log lines for a remote peer. */
class ILogSendService {
 public:
  virtual ~ILogSendService() = default;

  virtual logging::LogLevel getLevel() const = 0;

  virtual void setLevel(logging::LogLevel level) = 0;

  virtual void log(logging::LogLevel level, std::string&& msg) = 0;

  virtual void flush() = 0;
};

/** Bidirectional network log bridge. */
class INetLogService {
 public:
  virtual ~INetLogService() = default;

  virtual ILogReceiveService* logReceiver() = 0;

  virtual ILogSendService* logSender() = 0;

  /** Runs the log stream until \a stopFlag is set; default is no-op. */
  virtual void forwardUntil(std::atomic<bool>* stopFlag) {}
};

}  // namespace evrp::sdk
