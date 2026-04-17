#!/usr/bin/env sh
# 将 fmanlou/log（及依赖 fmt、spdlog）安装到 library/log、library/fmt、library/spdlog。
# 由 install-third-party-to-library.sh 调用；也可单独运行。
# 若 third_party/log 不存在，会从 LOG_GIT_URL 浅克隆（默认 https://github.com/fmanlou/log.git）。
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="$ROOT/library"
BASE="$ROOT/build/third-party"
FORCE="${EVRP_FORCE_THIRD_PARTY_INSTALL:-}"
LOG_GIT_URL="${LOG_GIT_URL:-https://github.com/fmanlou/log.git}"
FMT_REF="${FMT_REF:-10.2.1}"
FMT_REPO="${FMT_REPO:-https://github.com/fmtlib/fmt.git}"
SPDLOG_REF="${SPDLOG_REF:-v1.14.1}"
SPDLOG_REPO="${SPDLOG_REPO:-https://github.com/gabime/spdlog.git}"
LOG_SRC="$ROOT/third_party/log"

mkdir -p "$BASE"

fmt_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/fmt/lib/cmake/fmt/fmt-config.cmake" ] || \
    [ -f "$PREFIX/fmt/lib64/cmake/fmt/fmt-config.cmake" ]
}

spdlog_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/spdlog/lib/cmake/spdlog/spdlogConfig.cmake" ] || \
    [ -f "$PREFIX/spdlog/lib64/cmake/spdlog/spdlogConfig.cmake" ]
}

log_installed() {
  [ -n "$FORCE" ] && return 1
  [ -f "$PREFIX/log/lib/cmake/log/logConfig.cmake" ] || \
    [ -f "$PREFIX/log/lib64/cmake/log/logConfig.cmake" ]
}

if [ ! -f "$LOG_SRC/CMakeLists.txt" ]; then
  mkdir -p "$ROOT/third_party"
  echo "Cloning log into $LOG_SRC"
  git clone --depth 1 "$LOG_GIT_URL" "$LOG_SRC"
fi

if fmt_installed; then
  echo "Skip fmt: already present under $PREFIX/fmt"
else
  FMT_SRC="$BASE/fmt-src"
  FMT_BUILD="$BASE/fmt"
  rm -rf "$FMT_SRC" "$FMT_BUILD"
  git clone --depth 1 --branch "$FMT_REF" "$FMT_REPO" "$FMT_SRC"
  cmake -S "$FMT_SRC" -B "$FMT_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/fmt" \
    -DCMAKE_BUILD_TYPE=Release \
    -DFMT_TEST=OFF \
    "$@"
  cmake --build "$FMT_BUILD" --parallel
  cmake --install "$FMT_BUILD" --prefix "$PREFIX/fmt"
  rm -rf "$FMT_SRC" "$FMT_BUILD"
fi

if spdlog_installed; then
  echo "Skip spdlog: already present under $PREFIX/spdlog"
else
  SPDLOG_SRC="$BASE/spdlog-src"
  SPDLOG_BUILD="$BASE/spdlog"
  rm -rf "$SPDLOG_SRC" "$SPDLOG_BUILD"
  git clone --depth 1 --branch "$SPDLOG_REF" "$SPDLOG_REPO" "$SPDLOG_SRC"
  cmake -S "$SPDLOG_SRC" -B "$SPDLOG_BUILD" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX/spdlog" \
    -DCMAKE_PREFIX_PATH="$PREFIX/fmt" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPDLOG_BUILD_EXAMPLE=OFF \
    -DSPDLOG_BUILD_TESTS=OFF \
    -DSPDLOG_FMT_EXTERNAL=ON \
    "$@"
  cmake --build "$SPDLOG_BUILD" --parallel
  cmake --install "$SPDLOG_BUILD" --prefix "$PREFIX/spdlog"
  rm -rf "$SPDLOG_SRC" "$SPDLOG_BUILD"
fi

if log_installed; then
  echo "Skip log: already present under $PREFIX/log"
  exit 0
fi

LOG_BUILD="$BASE/log"
rm -rf "$LOG_BUILD"
cmake -S "$LOG_SRC" -B "$LOG_BUILD" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX/log" \
  -DCMAKE_PREFIX_PATH="$PREFIX/fmt;$PREFIX/spdlog" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLOG_BUILD_TESTS=OFF \
  "$@"
cmake --build "$LOG_BUILD" --parallel
cmake --install "$LOG_BUILD" --prefix "$PREFIX/log"
rm -rf "$LOG_BUILD"
