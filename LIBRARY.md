# `library/` — 第三方依赖安装目录

仓库根目录下的 **`library/`** 为**仓库内约定的第三方依赖安装根**，**不提交**具体库文件（见根目录 `.gitignore`）。

## 推荐布局

```
library/
  bin/          # 例如装到根前缀时的工具
  include/      # 装于根前缀时的头文件
  lib/          # 装于根前缀时的库与 CMake 包
  share/
  lua/          # Lua：lib/、include/lua/
  googletest/   # GoogleTest：lib/、include/、lib/cmake/GTest/
  protobuf/     # Protobuf：bin/protoc、include/、lib/（含 lib/cmake/protobuf）
  grpc/         # gRPC：bin/grpc_cpp_plugin、include/、lib/（含 lib/cmake/grpc）
  gflags/       # gflags：include/、lib/（含 lib/cmake/gflags）
  fmt/          # {fmt}：lib/、include/、lib/cmake/fmt/
  spdlog/       # spdlog：lib/、include/、lib/cmake/spdlog/
  log/          # fmanlou/log：lib/、include/log/…、lib/cmake/log/
```

根目录 **`CMakeLists.txt`** 已将 **`library/`**、**`library/protobuf/`**、**`library/grpc/`**、**`library/gflags/`**、**`library/fmt/`**、**`library/spdlog/`**、**`library/log/`** 加入 **`CMAKE_PREFIX_PATH`**；**gflags** 另用 **`find_package(gflags … PATHS library/gflags)`** 仅从本地安装查找；**log** 用 **`find_package(log … PATHS library/log)`**，其 **`logConfig.cmake`** 会通过 **`find_dependency`** 解析 **fmt** 与 **spdlog**。

## 第三方源码与安装

**`third_party/`** 下的 Lua、GoogleTest、Protobuf、gRPC、gflags 等为仓库内已包含的上游源码（非 Git 子模块）；若本地缺失目录，请自行从对应上游仓库获取对应版本后放入对应路径。

总入口 **`scripts/install-third-party-to-library.sh`** 会依次调用各库独立脚本；每个脚本在**安装成功后**会删除 **`build/third-party/<库名>/`** 临时编译目录。

也可**单独**安装某一库（同样支持跳过已安装目录、`EVRP_FORCE_THIRD_PARTY_INSTALL`）：

| 脚本 | 安装前缀 |
|------|----------|
| **`scripts/install-lua-to-library.sh`** | `library/lua/` |
| **`scripts/install-googletest-to-library.sh`** | `library/googletest/` |
| **`scripts/install-protobuf-to-library.sh`** | `library/protobuf/` |
| **`scripts/install-grpc-to-library.sh`** | `library/grpc/`（依赖已安装的 `library/protobuf`） |
| **`scripts/install-gflags-to-library.sh`** | `library/gflags/` |
| **`scripts/install-log-to-library.sh`** | `library/log/`（并安装依赖到 `library/fmt/`、`library/spdlog/`；若缺少 **`third_party/log`** 会从默认仓库浅克隆） |

一键安装到 **`library/`**：

```bash
./scripts/install-third-party-to-library.sh
```

（依次安装 Lua、GoogleTest、Protobuf、gRPC、gflags、log；其中 **log** 步骤会按需安装 **fmt、spdlog**；gRPC **使用已安装到 `library/protobuf` 的 Protobuf**。）若对应 **`library/<组件>/`** 下已有安装结果，会**跳过**该组件。若要**强制全部重装**：

```bash
EVRP_FORCE_THIRD_PARTY_INSTALL=1 ./scripts/install-third-party-to-library.sh
```

然后再在仓库根目录 `cmake -B build`。详见 [`docs/PROJECT_CONVENTIONS.md`](docs/PROJECT_CONVENTIONS.md)。

## 与 `third_party/` 的区别

| 路径 | 含义 |
|------|------|
| **`third_party/`** | **源码**（Lua、GoogleTest、Protobuf、gRPC、gflags 等） |
| **`library/`** | **安装前缀**：供主工程查找与链接 |
