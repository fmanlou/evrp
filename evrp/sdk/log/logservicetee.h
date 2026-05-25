#pragma once

#include <string>

#include "evrp/sdk/log/inetlogservice.h"
#include "log/common/ilogservice.h"

namespace evrp::sdk {

/** Writes each log line to a local sink and the network log sender. */
class LogServiceTee final : public logging::ILogService {
 public:
  LogServiceTee(logging::ILogService* primary, INetLogService* net);

  logging::LogLevel getLevel() const override;

  void setLevel(logging::LogLevel level) override;

  void log(logging::LogLevel level, std::string&& msg) override;

  void flush() override;

 private:
  logging::ILogService* primary_;
  INetLogService* net_;
};

}  // namespace evrp::sdk
