#!/usr/bin/env sh
# 在脚本内用 cmake 分别配置、编译、安装 Lua、GoogleTest、Protobuf、gRPC、gflags 到 <repo>/library/（不依赖仓库 cmake/ 下模块）。
# 用法：在仓库根目录执行
#   ./scripts/install-third-party-to-library.sh
# 可选：./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Debug（仅作用于各处 cmake -S）
#
# 若 library/<组件>/ 下已有对应安装结果，则跳过该组件的编译与安装。若要强制全部重装：
#   EVRP_FORCE_THIRD_PARTY_INSTALL=1 ./scripts/install-third-party-to-library.sh
#
# 子模块：脚本开头会执行 git submodule update --init --recursive（需在本仓库 git 工作区内执行）。
# gRPC 使用已安装到 library/protobuf 的 Protobuf（gRPC_PROTOBUF_PROVIDER=package）。

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PREFIX="$ROOT/library"
BASE="$ROOT/build-third-party-install"
LUA_BUILD="$BASE/lua"
GTEST_BUILD="$BASE/googletest"
PROTOBUF_BUILD="$BASE/protobuf"
GRPC_BUILD="$BASE/grpc"
GFLAGS_BUILD="$BASE/gflags"

mkdir -p "$BASE"

if [ -d "$ROOT/.git" ] || [ -f "$ROOT/.git" ]; then
  echo "git submodule update --init --recursive"
  git -C "$ROOT" submodule update --init --recursive
fi

# 非空则忽略「已安装」检测，始终编译安装各组件
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

# --- 已安装判定（与主工程 CMake 查找路径一致）---
lua_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/lua/include/lua/lua.h" ] || return 1
  for _f in "$PREFIX/lua/lib"/*.a; do
    [ -f "$_f" ] && return 0
  done
  return 1
}

gtest_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/googletest/lib/cmake/GTest/GTestConfig.cmake" ]
}

protobuf_installed() {
  [ -n "$FORCE" ] && return 1
  [ -x "$PREFIX/protobuf/bin/protoc" ] || return 1
  [ -f "$PREFIX/protobuf/lib/cmake/protobuf/protobuf-config.cmake" ] || \
    [ -f "$PREFIX/protobuf/lib64/cmake/protobuf/protobuf-config.cmake" ]
}

grpc_installed() {
  [ -n "$FORCE" ] && return 1
  [ -x "$PREFIX/grpc/bin/grpc_cpp_plugin" ] || return 1
  [ -f "$PREFIX/grpc/lib/cmake/grpc/gRPCConfig.cmake" ] || \
    [ -f "$PREFIX/grpc/lib64/cmake/grpc/gRPCConfig.cmake" ]
}

gflags_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/gflags/lib/cmake/gflags/gflags-config.cmake" ] || \
    [ -f "$PREFIX/gflags/lib64/cmake/gflags/gflags-config.cmake" ]
}

# Lua → library/lua/
if lua_installed; then
  echo "Skip Lua: already present under $PREFIX/lua"
else
  cmake -S "$ROOT/third_party/lua" -B "$LUA_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/lua" \
    "$@"
  cmake --build "$LUA_BUILD"
  cmake --install "$LUA_BUILD" --prefix "$PREFIX/lua"
fi

# GoogleTest → library/googletest/
if gtest_installed; then
  echo "Skip GoogleTest: already present under $PREFIX/googletest"
else
  cmake -S "$ROOT/third_party/googletest" -B "$GTEST_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/googletest" \
    -DBUILD_GMOCK=OFF \
    -DINSTALL_GTEST=ON \
    "$@"
  cmake --build "$GTEST_BUILD"
  cmake --install "$GTEST_BUILD" --prefix "$PREFIX/googletest"
fi

# Protobuf（含 protoc）→ library/protobuf/
if protobuf_installed; then
  echo "Skip Protobuf: already present under $PREFIX/protobuf"
else
  cmake -S "$ROOT/third_party/protobuf" -B "$PROTOBUF_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/protobuf" \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_SHARED_LIBS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    "$@"
  cmake --build "$PROTOBUF_BUILD" --parallel
  cmake --install "$PROTOBUF_BUILD" --prefix "$PREFIX/protobuf"
fi

# gRPC（含 grpc_cpp_plugin、grpc++）→ library/grpc/
# 依赖 third_party/grpc 内嵌套子模块（已由开头 git submodule 更新）
if grpc_installed; then
  echo "Skip gRPC: already present under $PREFIX/grpc"
else
  PROTOBUF_DIR="$PREFIX/protobuf/lib/cmake/protobuf"
  [ -d "$PROTOBUF_DIR" ] || PROTOBUF_DIR="$PREFIX/protobuf/lib64/cmake/protobuf"
  cmake -S "$ROOT/third_party/grpc" -B "$GRPC_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/grpc" \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_PROTOBUF_PROVIDER=package \
    -DCMAKE_PREFIX_PATH="$PREFIX/protobuf" \
    -DProtobuf_DIR="$PROTOBUF_DIR" \
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
    "$@"
  cmake --build "$GRPC_BUILD" --parallel
  cmake --install "$GRPC_BUILD" --prefix "$PREFIX/grpc"
fi

# gflags → library/gflags/
if gflags_installed; then
  echo "Skip gflags: already present under $PREFIX/gflags"
else
  cmake -S "$ROOT/third_party/gflags" -B "$GFLAGS_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/gflags" \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_STATIC_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    "$@"
  cmake --build "$GFLAGS_BUILD" --parallel
  cmake --install "$GFLAGS_BUILD" --prefix "$PREFIX/gflags"
fi

echo "Done. Installs under: $PREFIX/lua, $PREFIX/googletest, $PREFIX/protobuf, $PREFIX/grpc, $PREFIX/gflags"
echo "Configure: cmake -B build -DCMAKE_PREFIX_PATH=$PREFIX"
