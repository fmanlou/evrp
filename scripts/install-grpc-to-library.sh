#!/usr/bin/env sh
# 将 gRPC 安装到 <repo>/library/grpc/（使用已安装的 library/protobuf）。由 install-third-party-to-library.sh 调用，也可单独运行。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
GRPC_BUILD="$BASE/grpc"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

mkdir -p "$BASE"

grpc_installed() {
  [ -n "$FORCE" ] && return 1
  [ -x "$PREFIX/grpc/bin/grpc_cpp_plugin" ] || return 1
  [ -f "$PREFIX/grpc/lib/cmake/grpc/gRPCConfig.cmake" ] || \
    [ -f "$PREFIX/grpc/lib64/cmake/grpc/gRPCConfig.cmake" ]
}

if grpc_installed; then
  echo "Skip gRPC: already present under $PREFIX/grpc"
  exit 0
fi

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
rm -rf "$GRPC_BUILD"
