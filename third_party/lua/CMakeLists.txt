cmake_minimum_required(VERSION 3.14)
project(lua C)

set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LUA_SOURCES
  lapi.c lauxlib.c lbaselib.c lcode.c lcorolib.c lctype.c
  ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c
  liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c
  lopcodes.c loslib.c lparser.c lstate.c lstring.c lstrlib.c
  ltable.c ltablib.c ltm.c lundump.c lutf8lib.c lvm.c lzio.c
)
list(TRANSFORM LUA_SOURCES PREPEND ${LUA_DIR}/)

add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC ${LUA_DIR})
target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
target_link_libraries(lua PUBLIC m)

# 安装到 <CMAKE_INSTALL_PREFIX>/lua/（默认前缀为 <repo>/library），供后续构建复用 IMPORTED 库
install(
  TARGETS lua
  ARCHIVE DESTINATION lua/lib
  LIBRARY DESTINATION lua/lib)
install(
  FILES ${LUA_DIR}/lua.h ${LUA_DIR}/lauxlib.h ${LUA_DIR}/lualib.h ${LUA_DIR}/luaconf.h
  DESTINATION lua/include/lua)
