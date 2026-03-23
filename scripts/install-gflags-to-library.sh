#!/usr/bin/env sh
# 将 gflags 安装到 <repo>/library/gflags/。由 install-third-party-to-library.sh 调用，也可单独运行。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
GFLAGS_BUILD="$BASE/gflags"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

mkdir -p "$BASE"

gflags_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/gflags/lib/cmake/gflags/gflags-config.cmake" ] || \
    [ -f "$PREFIX/gflags/lib64/cmake/gflags/gflags-config.cmake" ]
}

if gflags_installed; then
  echo "Skip gflags: already present under $PREFIX/gflags"
  exit 0
fi

cmake -S "$ROOT/third_party/gflags" -B "$GFLAGS_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/gflags" \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_TESTING=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  "$@"
cmake --build "$GFLAGS_BUILD" --parallel
cmake --install "$GFLAGS_BUILD" --prefix "$PREFIX/gflags"
rm -rf "$GFLAGS_BUILD"
