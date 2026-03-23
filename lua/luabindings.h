#pragma once

// Lua C API bindings for evrp.
// Include this when embedding Lua. Link with target 'lua'.

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

class InputEventWriter;

namespace evrp {
namespace lua {

// Run a Lua script file. Returns LUA_OK on success.
int run_script(const char* path);

// Execute a Lua chunk (e.g. a line of code) with the given writer.
// Uses the same keyboard/mouse API as run_script. Returns LUA_OK on success.
int execute_chunk(InputEventWriter* writer, const char* chunk);

}  // namespace lua
}  // namespace evrp
