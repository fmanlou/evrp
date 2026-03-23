# 项目规范

## 第三方依赖与目录约定

### 1. `library/` — 第三方**安装根目录**

- 所有**非本仓库源码自带**、需单独编译或预编译安装的第三方库，**统一安装到仓库根目录下的 `library/`**。

- 约定布局：

  | 子目录 | 用途 |
  |--------|------|
  | `library/bin` | 可执行工具（如 `grpc_cpp_plugin` 等装于根前缀时） |
  | `library/include` / `library/lib` | 装于根前缀时的头文件与 CMake 包 |
  | `library/protobuf/` | Protobuf：`bin/protoc`、`include/`、`lib/`（含 `lib/cmake/protobuf`） |
  | `library/share` | 按需（部分工具链使用） |

- 顶层 **`CMakeLists.txt`** 将 **`EVRP_LIBRARY_DIR`** 与 **`EVRP_LIBRARY_DIR/protobuf`** 加入 **`CMAKE_PREFIX_PATH`**，以便 **`find_package(Protobuf)`** 发现 `library/protobuf/lib/cmake/...`；Lua/GTest 仍用显式路径；**`find_program(protoc …)`** 在 **`library/protobuf/bin`**（及 **`library/bin`**）中查找。

- 安装到 `library/` 的**具体文件**不纳入版本控制（见根目录 `.gitignore`）；目录说明见仓库根目录 **`LIBRARY.md`**。

### 2. Lua / GoogleTest / Protobuf：主工程**仅从 `library/` 获取（或 PREFIX 解析）**

- 根 **`CMakeLists.txt`** 只在 **`library/`** 下查找 Lua 与 GoogleTest（`NO_DEFAULT_PATH`）；**Protobuf** 通过 **`CMAKE_PREFIX_PATH`**（已包含 **`library/protobuf`**）由 **`find_package(Protobuf)`** 解析，不编译 `third_party` 源码。
- 配置主工程 **前**，须已把 Lua / GTest 安装到对应子目录，**Protobuf**（含 **`protoc`**）安装到 **`library/protobuf/`**（由安装脚本写入）。
- **`third_party/lua`、`third_party/googletest`、`third_party/protobuf`** 为子模块源码；**编译与安装**由 **`scripts/install-third-party-to-library.sh`** 完成（脚本内直接 `cmake -S third_party/...`）。**`third_party/protobuf` 内的嵌套子模块（abseil-cpp 等）**需在克隆/维护流程中自行 **`git submodule update --init --recursive`**（或等价方式），安装脚本不负责拉取。

**克隆仓库后请先拉取子模块（含 `third_party/protobuf`）：**

```bash
git submodule update --init --recursive
```

**首次或更新第三方源码后：**

```bash
./scripts/install-third-party-to-library.sh
cmake -B build
cmake --build build
```

可选向脚本传入额外 CMake 参数（会传给 Lua、GTest、Protobuf 三处 `cmake -S`），例如：

```bash
./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Release
```

若 `library/` 中缺少 Lua 或 GTest，主工程 `cmake` 会 **FATAL_ERROR** 并提示运行上述脚本；缺少 **Protobuf** 时 **`find_package(Protobuf REQUIRED)`** 会失败，同样需先运行安装脚本（或自行安装到 **`library/protobuf/`**）。

**故障排除**：Lua 路径类错误多为旧缓存：删除 `build/` 后重新 `cmake -B build`。误装到 **`/usr/local/googletest`** 时可：`sudo rm -rf /usr/local/googletest`。

### 3. 与 evrp-device 相关依赖

- **Protobuf**（含 **`protoc`**）安装到 **`library/protobuf/`**；**gRPC C++、gflags** 及 **`grpc_cpp_plugin`** 等安装到 **`library/`** 约定路径后，再运行本仓库 `cmake` 配置。

- 自行从源码构建时，示例：

  ```bash
  cmake -S <grpc-source> -B grpc-build -DCMAKE_INSTALL_PREFIX="$PWD/library"
  cmake --build grpc-build --target install
  ```

  （具体工程与目标名以对应上游文档为准。）

- `library/` 目录说明：[`../LIBRARY.md`](../LIBRARY.md)
