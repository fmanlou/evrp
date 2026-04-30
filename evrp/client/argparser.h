#pragma once

#include <string>

#include "evrp/device/api/types.h"
#include "evrp/sdk/stringkeystore.h"

void printUsage(const char* prog);
bool parseKind(const std::string& s, evrp::device::api::DeviceKind* outKind);

/// Fills `options` via `insert` from argv (gflags + legacy flags). Keys/types:
/// program (string), recording (bool), playback (bool),
/// logLevel (logging::LogLevel), playbackPath, outputPath,
/// device (string): empty → UDP discovery; else host:port for direct gRPC;
/// kinds (vector<DeviceKind>), executeWaitBeforeFirst, executeWaitAfterLast (bool).
void parseArgvInto(StringKeyStore& options, int argc, char* argv[]);
