#pragma once

#include <gflags/gflags.h>
#include <string>

#include "evrp/sdk/setting/isetting.h"

DECLARE_string(host);

void printUsage(const char* prog);

void parseArgvInto(ISetting& options, int argc, char* argv[]);
