#!/usr/bin/env sh
# 将 Protobuf（含 protoc）安装到 <repo>/library/protobuf/。由 install-third-party-to-library.sh 调用，也可单独运行。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
PROTOBUF_BUILD="$BASE/protobuf"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

mkdir -p "$BASE"

protobuf_installed() {
  [ -n "$FORCE" ] && return 1
  [ -x "$PREFIX/protobuf/bin/protoc" ] || return 1
  [ -f "$PREFIX/protobuf/lib/cmake/protobuf/protobuf-config.cmake" ] || \
    [ -f "$PREFIX/protobuf/lib64/cmake/protobuf/protobuf-config.cmake" ]
}

if protobuf_installed; then
  echo "Skip Protobuf: already present under $PREFIX/protobuf"
  exit 0
fi

cmake -S "$ROOT/third_party/protobuf" -B "$PROTOBUF_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/protobuf" \
  -Dprotobuf_BUILD_TESTS=OFF \
  -Dprotobuf_BUILD_SHARED_LIBS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  "$@"
cmake --build "$PROTOBUF_BUILD" --parallel
cmake --install "$PROTOBUF_BUILD" --prefix "$PREFIX/protobuf"
rm -rf "$PROTOBUF_BUILD"
