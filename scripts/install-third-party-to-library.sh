#!/usr/bin/env sh
# 依次调用各库独立安装脚本，将 Lua、GoogleTest、Protobuf、gRPC、gflags、Asio、log（及 fmt/spdlog）安装到 <repo>/library/。
# 用法：在仓库根目录执行
#   ./scripts/install-third-party-to-library.sh
# 可选：./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Debug（传给各子脚本中 cmake -S）
#
# 若 library/<组件>/ 下已有对应安装结果，各子脚本会跳过；强制重装：EVRP_FORCE_THIRD_PARTY_INSTALL=1
#
# 约定：CMake 构建树在 build/third-party/<库名>/（与 lua、gflags、grpc、asio、fmt、spdlog、log 等一致）；
# fmt/spdlog 等临时浅克隆在 build/third-party/<库名>-src/，成功后删除；Asio 上游在 third_party/asio/（克隆，见 .gitignore）。

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
SCR="$ROOT/scripts"

"$SCR/install-lua-to-library.sh" "$@"
"$SCR/install-googletest-to-library.sh" "$@"
"$SCR/install-protobuf-to-library.sh" "$@"
"$SCR/install-grpc-to-library.sh" "$@"
"$SCR/install-gflags-to-library.sh" "$@"
"$SCR/install-asio-to-library.sh" "$@"
"$SCR/install-log-to-library.sh" "$@"

echo "Done. Installs under: $ROOT/library/lua, $ROOT/library/googletest, $ROOT/library/protobuf, $ROOT/library/grpc, $ROOT/library/gflags, $ROOT/library/asio, $ROOT/library/fmt, $ROOT/library/spdlog, $ROOT/library/log"
echo "Configure: cmake -B build -DCMAKE_PREFIX_PATH=$ROOT/library"
