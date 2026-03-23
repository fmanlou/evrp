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
```

根目录 **`CMakeLists.txt`** 已将 **`library/`**、**`library/protobuf/`**、**`library/grpc/`**、**`library/gflags/`** 加入 **`CMAKE_PREFIX_PATH`**；**gflags** 另用 **`find_package(gflags … PATHS library/gflags)`** 仅从本地安装查找。

## 子模块与安装

在仓库根目录**一次性**拉取并更新所有子模块（含嵌套）：

```bash
git submodule update --init --recursive
```

安装脚本 **`scripts/install-third-party-to-library.sh`** 开头也会执行上述命令（需在 **git 克隆** 的工作目录下运行）。

再安装到 **`library/`**：

```bash
./scripts/install-third-party-to-library.sh
```

（脚本依次安装 Lua、GoogleTest、Protobuf、gRPC、gflags；gRPC **使用已安装到 `library/protobuf` 的 Protobuf**。）若 **`library/lua`**、**`library/googletest`**、**`library/protobuf`**、**`library/grpc`**、**`library/gflags`** 下已有对应安装结果，会**跳过**该组件的编译与安装。若要**强制全部重装**：

```bash
EVRP_FORCE_THIRD_PARTY_INSTALL=1 ./scripts/install-third-party-to-library.sh
```

然后再在仓库根目录 `cmake -B build`。详见 [`docs/PROJECT_CONVENTIONS.md`](docs/PROJECT_CONVENTIONS.md)。

## 与 `third_party/` 的区别

| 路径 | 含义 |
|------|------|
| **`third_party/`** | **源码**（Lua、GoogleTest、Protobuf、gRPC、gflags 等） |
| **`library/`** | **安装前缀**：供主工程查找与链接 |
