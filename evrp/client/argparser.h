#pragma once

#include <any>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/logger.h"

/// Holds string keys and `std::any` values from command-line parsing, with a
/// typed template accessor.
class ParsedOptions {
 public:
  ParsedOptions() = default;

  bool contains(const std::string& key) const {
    return values_.find(key) != values_.end();
  }

  template <typename T>
  T get(const std::string& key, T defaultValue) const {
    auto it = values_.find(key);
    if (it == values_.end()) return defaultValue;
    if (auto* p = std::any_cast<T>(&it->second)) return *p;
    return defaultValue;
  }

  template <typename T>
  T get(const std::string& key) const {
    return get(key, T{});
  }

  void insert(std::string key, std::any value) {
    values_.insert_or_assign(std::move(key), std::move(value));
  }

  template <typename T>
  void insert(std::string key, T&& value) {
    values_.insert_or_assign(std::move(key),
                             std::any(std::forward<T>(value)));
  }

 private:
  std::map<std::string, std::any> values_;
};

void printUsage(const char* prog);
bool parseKind(const std::string& s, evrp::device::api::DeviceKind* outKind);

/// Fills `options` via `insert` from argv (gflags + legacy flags). Keys/types:
/// program (string), recording (bool), playback (bool),
/// logLevel (logging::LogLevel), playbackPath, outputPath, device (string),
/// kinds (vector<DeviceKind>), executeWaitBeforeFirst, executeWaitAfterLast (bool).
void parseArgvInto(ParsedOptions& options, int argc, char* argv[]);
