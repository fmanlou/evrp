#include "lua_bindings.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <script.lua>\n";
    return 1;
  }
  int err = evrp::lua::run_script(argv[1]);
  return (err == LUA_OK) ? 0 : 1;
}
