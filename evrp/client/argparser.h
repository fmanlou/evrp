#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/logger.h"

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

void printUsage(const char* prog);
bool parseKind(const std::string& s, evrp::device::api::DeviceKind* outKind);

/// Keys and stored types: program (string), recording (bool), playback (bool),
/// logLevel (logging::LogLevel), playbackPath, outputPath, device (string),
/// kinds (vector<DeviceKind>), executeWaitBeforeFirst, executeWaitAfterLast (bool).
std::map<std::string, std::any> parseOptions(int argc, char* argv[]);
