#pragma once

// Lua C API bindings for evrp.
// Include this when embedding Lua. Link with target 'lua'.

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace evrp {
namespace lua {

// Run a Lua script file. Returns LUA_OK on success.
int run_script(const char* path);

}  // namespace lua
}  // namespace evrp
