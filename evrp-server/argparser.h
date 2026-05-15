#pragma once

#include <gflags/gflags.h>
#include <string>

DECLARE_string(host);
DECLARE_string(listen);
DECLARE_string(log_level);

#include "evrp/sdk/setting/isetting.h"

void printUsage(const char* prog);

void parseArgvInto(ISetting& options, int argc, char* argv[]);
