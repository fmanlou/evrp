#!/usr/bin/env sh
# 将 Lua 安装到 <repo>/library/lua/。由 install-third-party-to-library.sh 调用，也可单独运行。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
LUA_BUILD="$BASE/lua"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

mkdir -p "$BASE"

lua_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/lua/include/lua/lua.h" ] || return 1
  for _f in "$PREFIX/lua/lib"/*.a; do
    [ -f "$_f" ] && return 0
  done
  return 1
}

if lua_installed; then
  echo "Skip Lua: already present under $PREFIX/lua"
  exit 0
fi

cmake -S "$ROOT/third_party/lua" -B "$LUA_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/lua" \
  "$@"
cmake --build "$LUA_BUILD"
cmake --install "$LUA_BUILD" --prefix "$PREFIX/lua"
rm -rf "$LUA_BUILD"
