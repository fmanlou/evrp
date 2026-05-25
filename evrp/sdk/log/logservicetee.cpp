#include "evrp/sdk/log/logservicetee.h"

namespace evrp::sdk {

LogServiceTee::LogServiceTee(logging::ILogService* primary, INetLogService* net)
    : logging::ILogService("LogServiceTee"), primary_(primary), net_(net) {}

logging::LogLevel LogServiceTee::getLevel() const {
  return primary_ ? primary_->getLevel() : logging::LogLevel::Info;
}

void LogServiceTee::setLevel(logging::LogLevel level) {
  if (primary_) {
    primary_->setLevel(level);
  }
  if (net_) {
    if (ILogSendService* sender = net_->logSender()) {
      sender->setLevel(level);
    }
    if (ILogReceiveService* receiver = net_->logReceiver()) {
      receiver->setLevel(level);
    }
  }
}

void LogServiceTee::log(logging::LogLevel level, std::string&& msg) {
  if (net_) {
    if (ILogSendService* sender = net_->logSender()) {
      sender->log(level, std::string(msg));
    }
  }
  if (primary_) {
    primary_->log(level, std::move(msg));
  }
}

void LogServiceTee::flush() {
  if (primary_) {
    primary_->flush();
  }
  if (net_) {
    if (ILogSendService* sender = net_->logSender()) {
      sender->flush();
    }
    if (ILogReceiveService* receiver = net_->logReceiver()) {
      receiver->flush();
    }
  }
}

}  // namespace evrp::sdk
