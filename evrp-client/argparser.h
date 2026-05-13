#pragma once

#include <string>

#include "evrp/sdk/setting/isetting.h"

void printUsage(const char* prog);

void parseArgvInto(ISetting& options, int argc, char* argv[]);
