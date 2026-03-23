#!/usr/bin/env sh
# 在脚本内用 cmake 分别配置、编译、安装 Lua、GoogleTest、Protobuf 到 <repo>/library/（不依赖仓库 cmake/ 下模块）。
# 用法：在仓库根目录执行
#   ./scripts/install-third-party-to-library.sh
# 可选：./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Debug（仅作用于各处 cmake -S）
#
# Protobuf 所依赖的 third_party/protobuf 嵌套子模块（abseil 等）默认在仓库外或克隆时自行处理，本脚本不执行 git submodule。

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PREFIX="$ROOT/library"
BASE="$ROOT/build-third-party-install"
LUA_BUILD="$BASE/lua"
GTEST_BUILD="$BASE/googletest"
PROTOBUF_BUILD="$BASE/protobuf"

mkdir -p "$BASE"

# Lua → library/lua/
cmake -S "$ROOT/third_party/lua" -B "$LUA_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/lua" \
  "$@"
cmake --build "$LUA_BUILD"
cmake --install "$LUA_BUILD" --prefix "$PREFIX/lua"

# GoogleTest → library/googletest/
cmake -S "$ROOT/third_party/googletest" -B "$GTEST_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/googletest" \
  -DBUILD_GMOCK=OFF \
  -DINSTALL_GTEST=ON \
  "$@"
cmake --build "$GTEST_BUILD"
cmake --install "$GTEST_BUILD" --prefix "$PREFIX/googletest"

# Protobuf（含 protoc）→ library/protobuf/
cmake -S "$ROOT/third_party/protobuf" -B "$PROTOBUF_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/protobuf" \
  -Dprotobuf_BUILD_TESTS=OFF \
  -Dprotobuf_BUILD_SHARED_LIBS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  "$@"
cmake --build "$PROTOBUF_BUILD" --parallel
cmake --install "$PROTOBUF_BUILD" --prefix "$PREFIX/protobuf"

echo "Installed: $PREFIX/lua, $PREFIX/googletest, $PREFIX/protobuf"
echo "Configure: cmake -B build -DCMAKE_PREFIX_PATH=$PREFIX"
