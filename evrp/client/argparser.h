#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/logger.h"

struct RunOptions {
  bool recording;
  bool playback;
  logging::LogLevel logLevel;
  std::string playbackPath;
  std::string outputPath;
  /// gRPC target for evrp-device (host:port).
  std::string device;
  std::vector<evrp::device::api::DeviceKind> kinds;

  bool executeWaitBeforeFirst;
  bool executeWaitAfterLast;

  /// String-keyed snapshot of fields above after `parseOptions` (types match each field).
  std::map<std::string, std::any> parsed;
};

namespace parsed_options {

std::string stringOr(const std::map<std::string, std::any>& m,
                     const std::string& key, std::string defaultValue = {});

bool boolOr(const std::map<std::string, std::any>& m, const std::string& key,
            bool defaultValue = false);

logging::LogLevel logLevelOr(const std::map<std::string, std::any>& m,
                             const std::string& key,
                             logging::LogLevel defaultValue = logging::LogLevel::Info);

std::vector<evrp::device::api::DeviceKind> kindsOr(
    const std::map<std::string, std::any>& m, const std::string& key,
    std::vector<evrp::device::api::DeviceKind> defaultValue = {});

}  // namespace parsed_options

void printUsage(const char *prog);
bool parseKind(const std::string &s, evrp::device::api::DeviceKind *outKind);
RunOptions parseOptions(int argc, char *argv[]);
