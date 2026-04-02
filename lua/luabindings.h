#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

class InputEventWriter;

namespace evrp {
namespace lua {

int runScript(const char* path);

int executeChunk(InputEventWriter* writer, const char* chunk);

}
}
