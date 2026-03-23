# `library/` — 第三方依赖安装目录

仓库根目录下的 **`library/`** 为**仓库内约定的第三方依赖安装根**，**不提交**具体库文件（见根目录 `.gitignore`）。

## 推荐布局

```
library/
  bin/          # 例如 grpc_cpp_plugin（若装到根前缀）
  include/      # 装于根前缀时的头文件
  lib/          # 装于根前缀时的库与 CMake 包
  share/
  lua/          # Lua：lib/、include/lua/
  googletest/   # GoogleTest：lib/、include/、lib/cmake/GTest/
  protobuf/     # Protobuf：bin/protoc、include/、lib/（含 lib/cmake/protobuf）
```

根目录 **`CMakeLists.txt`** 已将 **`library/`** 与 **`library/protobuf/`** 加入 **`CMAKE_PREFIX_PATH`**，**Protobuf** 可被 **`find_package(Protobuf)`** 找到。将 **gRPC C++、gflags** 等同样安装到 **`library/`** 后，`find_package(gRPC)` 等即可解析。

## Lua / GoogleTest / Protobuf

请先拉取子模块（含 **`third_party/protobuf`**；Protobuf 目录内嵌套的 abseil 等需在同一命令中一并初始化）：

```bash
git submodule update --init --recursive
```

再安装到 **`library/`**：

```bash
./scripts/install-third-party-to-library.sh
```

（脚本在 **`scripts/`** 内完成 Lua、GoogleTest、Protobuf 的 `cmake` 配置、编译与安装，不依赖仓库 `cmake/` 目录。）然后再在仓库根目录 `cmake -B build`。详见 [`docs/PROJECT_CONVENTIONS.md`](docs/PROJECT_CONVENTIONS.md)。

## 与 `third_party/` 的区别

| 路径 | 含义 |
|------|------|
| **`third_party/`** | **源码**（Lua、GoogleTest 等） |
| **`library/`** | **安装前缀**：供主工程查找与链接 |
