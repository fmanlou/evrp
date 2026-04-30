#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/logger.h"

/// Holds string keys and `std::any` values from `parseOptions`, with typed accessors.
class ParsedOptions {
 public:
  ParsedOptions() = default;
  explicit ParsedOptions(std::map<std::string, std::any> values);

  std::string stringOr(const std::string& key, std::string defaultValue = {}) const;
  bool boolOr(const std::string& key, bool defaultValue = false) const;
  logging::LogLevel logLevelOr(
      const std::string& key,
      logging::LogLevel defaultValue = logging::LogLevel::Info) const;
  std::vector<evrp::device::api::DeviceKind> kindsOr(
      const std::string& key,
      std::vector<evrp::device::api::DeviceKind> defaultValue = {}) const;

  const std::map<std::string, std::any>& map() const { return values_; }

 private:
  std::map<std::string, std::any> values_;
};

void printUsage(const char* prog);
bool parseKind(const std::string& s, evrp::device::api::DeviceKind* outKind);

/// Keys and stored types: program (string), recording (bool), playback (bool),
/// logLevel (logging::LogLevel), playbackPath, outputPath, device (string),
/// kinds (vector<DeviceKind>), executeWaitBeforeFirst, executeWaitAfterLast (bool).
ParsedOptions parseOptions(int argc, char* argv[]);
