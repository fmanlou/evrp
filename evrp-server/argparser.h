#pragma once

#include <gflags/gflags.h>
#include <string>

#include "evrp/sdk/setting/isetting.h"

DECLARE_string(listen);
DECLARE_string(log_level);

void parseServerArgvInto(ISetting& options, int argc, char* argv[]);
