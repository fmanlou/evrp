#pragma once

#include <string>

#include "evrp/device/api/types.h"
#include "evrp/sdk/setting/isetting.h"

void printUsage(const char* prog);
evrp::device::api::DeviceKind toKind(const std::string& s);
void toKind(const std::string& s, evrp::device::api::DeviceKind* outKind);

/// Fills `options` via `insert` from argv (gflags + legacy flags). Keys/types:
/// program (string), recording (bool), playback (bool),
/// logLevel (logging::LogLevel), playbackPath, outputPath,
/// device (string): empty → UDP discovery; else host:port for direct gRPC;
/// kinds (vector<DeviceKind>): comma-separated `--kind` when recording; if
/// omitted while recording, all four device kinds.
/// executeWaitBeforeFirst / executeWaitAfterLast (bool).
/// keyboardCtrlCFilter (string): off | full | ending (recording keyboard;
/// default ending).
void parseArgvInto(ISetting& options, int argc, char* argv[]);
