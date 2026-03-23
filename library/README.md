# `library/` — 第三方依赖安装目录

本目录为**仓库内约定的第三方依赖安装根**，**不提交**具体库文件（见根目录 `.gitignore`）。

## 推荐布局

```
library/
  bin/          # protoc、grpc_cpp_plugin 等
  include/      # 头文件（gRPC / Protobuf 等）
  lib/          # 库与 CMake 包（*.cmake）
  share/
  lua/          # Lua：lib/、include/lua/
  googletest/   # GoogleTest：lib/、include/、lib/cmake/GTest/
```

将 **Protobuf、gRPC C++、gflags** 等安装到上述路径后，配置 CMake 时建议加上 **`-DCMAKE_PREFIX_PATH=<repo>/library`**，以便 `find_package` 找到它们。

## Lua / GoogleTest

主工程只从 **`library/`** 使用 Lua 与 GoogleTest。请先执行：

```bash
./scripts/install-third-party-to-library.sh
```

（脚本在 **`scripts/`** 内完成 `cmake` 配置、编译与安装，不依赖仓库 `cmake/` 目录。）然后再在仓库根目录 `cmake -B build`。详见 [`docs/PROJECT_CONVENTIONS.md`](../docs/PROJECT_CONVENTIONS.md)。

## 与 `third_party/` 的区别

| 路径 | 含义 |
|------|------|
| **`third_party/`** | **源码**（Lua、GoogleTest 等） |
| **`library/`** | **安装前缀**：供主工程查找与链接 |
