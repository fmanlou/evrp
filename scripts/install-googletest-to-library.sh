#!/usr/bin/env sh
# 将 GoogleTest 安装到 <repo>/library/googletest/。由 install-third-party-to-library.sh 调用，也可单独运行。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
GTEST_BUILD="$BASE/googletest"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"

mkdir -p "$BASE"

gtest_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/googletest/lib/cmake/GTest/GTestConfig.cmake" ]
}

if gtest_installed; then
  echo "Skip GoogleTest: already present under $PREFIX/googletest"
  exit 0
fi

cmake -S "$ROOT/third_party/googletest" -B "$GTEST_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/googletest" \
  -DBUILD_GMOCK=OFF \
  -DINSTALL_GTEST=ON \
  "$@"
cmake --build "$GTEST_BUILD"
cmake --install "$GTEST_BUILD" --prefix "$PREFIX/googletest"
rm -rf "$GTEST_BUILD"
