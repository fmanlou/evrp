#!/usr/bin/env sh
# 在脚本内用 cmake 分别配置、编译、安装 Lua 与 GoogleTest 到 <repo>/library/（不依赖仓库 cmake/ 下模块）。
# 用法：在仓库根目录执行
#   ./scripts/install-third-party-to-library.sh
# 可选：./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Debug（仅作用于两处 cmake -S）

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PREFIX="$ROOT/library"
BASE="$ROOT/build-third-party-install"
LUA_BUILD="$BASE/lua"
GTEST_BUILD="$BASE/googletest"

mkdir -p "$BASE"

# Lua → library/lua/{lib,include/lua}
cmake -S "$ROOT/third_party/lua" -B "$LUA_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  "$@"
cmake --build "$LUA_BUILD"
cmake --install "$LUA_BUILD" --prefix "$PREFIX"

# GoogleTest → library/googletest/...
cmake -S "$ROOT/third_party/googletest" -B "$GTEST_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DBUILD_GMOCK=OFF \
  -DINSTALL_GTEST=ON \
  -DCMAKE_INSTALL_LIBDIR=googletest/lib \
  -DCMAKE_INSTALL_INCLUDEDIR=googletest/include \
  "$@"
cmake --build "$GTEST_BUILD"
cmake --install "$GTEST_BUILD" --prefix "$PREFIX"

echo "Installed under $PREFIX/lua and $PREFIX/googletest — run: cmake -B build (repo root)"
