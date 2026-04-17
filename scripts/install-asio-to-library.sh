#!/usr/bin/env sh
# 将 Standalone Asio 安装到 <repo>/library/asio/：include/（上游头文件）、
# lib/libasio_separate.a（third_party/asioimpl 工程在脚本内 cmake 编译）。
# 上游仓库克隆到 third_party/asio/（与 third_party/log 同类，不提交）。
# 由 install-third-party-to-library.sh 调用，也可单独运行。
# 版本：ASIO_REF（默认 asio-1-30-2）；仓库：ASIO_REPO。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"
ASIO_REF="${ASIO_REF:-asio-1-30-2}"
ASIO_REPO="${ASIO_REPO:-https://github.com/chriskohlhoff/asio.git}"
ASIO_UPSTREAM="$ROOT/third_party/asio"
ASIO_BUILD="$BASE/asio"
EVRP_ASIO_IMPL="$ROOT/third_party/asioimpl/asioimpl.cpp"
EVRP_ASIO_CMAKE="$ROOT/third_party/asioimpl/CMakeLists.txt"

mkdir -p "$BASE"
mkdir -p "$ROOT/third_party"

# 旧版曾使用 build/third-party/asio-src，清理遗留目录
rm -rf "$BASE/asio-src"

if [ ! -f "$EVRP_ASIO_IMPL" ] || [ ! -f "$EVRP_ASIO_CMAKE" ]; then
  echo "Missing third_party/asioimpl (asioimpl.cpp and CMakeLists.txt)" >&2
  exit 1
fi

asio_complete() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/asio/include/asio.hpp" ] && {
    [ -f "$PREFIX/asio/lib/libasio_separate.a" ] || \
      [ -f "$PREFIX/asio/lib64/libasio_separate.a" ]
  }
}

if asio_complete; then
  echo "Skip asio: already present under $PREFIX/asio"
  exit 0
fi

if [ ! -f "$PREFIX/asio/include/asio.hpp" ] || [ -n "$FORCE" ]; then
  if [ -n "$FORCE" ] || [ ! -d "$ASIO_UPSTREAM/.git" ]; then
    rm -rf "$ASIO_UPSTREAM"
    git clone --depth 1 --branch "$ASIO_REF" "$ASIO_REPO" "$ASIO_UPSTREAM"
  fi
  mkdir -p "$PREFIX/asio/include"
  cp -a "$ASIO_UPSTREAM/asio/include/." "$PREFIX/asio/include/"
fi

rm -rf "$PREFIX/asio/src"

rm -rf "$ASIO_BUILD"
cmake -S "$ROOT/third_party/asioimpl" -B "$ASIO_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/asio" \
  "$@"
cmake --build "$ASIO_BUILD" --parallel
cmake --install "$ASIO_BUILD" --prefix "$PREFIX/asio"
rm -rf "$ASIO_BUILD"
