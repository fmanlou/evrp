#include "lua_bindings.h"

#include <cstdio>
#include <cstdlib>

namespace evrp {
namespace lua {

// Minimal Lua integration - run a script file.
// Returns 0 on success, non-zero on error.
int run_script(const char* path) {
  lua_State* L = luaL_newstate();
  if (!L) return -1;
  luaL_openlibs(L);
  int err = luaL_dofile(L, path);
  if (err != LUA_OK) {
    std::fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_close(L);
  return err;
}

}  // namespace lua
}  // namespace evrp
