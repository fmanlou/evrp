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
  | `library/grpc/` | gRPC：`bin/grpc_cpp_plugin`、`include/`、`lib/`（含 `lib/cmake/grpc`） |
  | `library/gflags/` | gflags：`include/`、`lib/`（含 `lib/cmake/gflags`） |
  | `library/share` | 按需（部分工具链使用） |

- 顶层 **`CMakeLists.txt`** 将 **`EVRP_LIBRARY_DIR`**、**`EVRP_LIBRARY_DIR/protobuf`**、**`EVRP_LIBRARY_DIR/grpc`**、**`EVRP_LIBRARY_DIR/gflags`** 加入 **`CMAKE_PREFIX_PATH`**，以便 **`find_package(Protobuf)`** / **`find_package(gRPC)`** 等发现对应 `lib/cmake/...`；**`find_package(gflags)`** 使用 **`PATHS library/gflags`**；Lua/GTest 仍用显式路径；**`find_program(protoc …)`**、**`find_program(grpc_cpp_plugin …)`** 在 **`library/protobuf/bin`**、**`library/grpc/bin`**（及 **`library/bin`**）中查找。

- 安装到 `library/` 的**具体文件**不纳入版本控制（见根目录 `.gitignore`）；目录说明见仓库根目录 **`LIBRARY.md`**。

### 2. Lua / GoogleTest / Protobuf / gRPC / gflags：主工程**仅从 `library/` 获取（或 PREFIX 解析）**

- 根 **`CMakeLists.txt`** 只在 **`library/`** 下查找 Lua 与 GoogleTest（`NO_DEFAULT_PATH`）；**Protobuf**、**gRPC**、**gflags** 通过 **`CMAKE_PREFIX_PATH`** / **`find_package(... PATHS)`** 从 **`library/protobuf`**、**`library/grpc`**、**`library/gflags`** 解析，不编译 `third_party` 源码。
- 配置主工程 **前**，须已把 Lua / GTest / Protobuf / gRPC / gflags 安装到对应子目录（由安装脚本写入）。
- **`third_party/lua`、`third_party/googletest`、`third_party/protobuf`、`third_party/grpc`、`third_party/gflags`** 为随仓库提供的上游源码树；**编译与安装**由 **`scripts/install-third-party-to-library.sh`**（总入口）及各库脚本 **`install-*-to-library.sh`** 完成（**gRPC 使用已安装到 `library/protobuf` 的 Protobuf**）。详见 **`LIBRARY.md`**。

**首次或更新第三方源码后：**

```bash
./scripts/install-third-party-to-library.sh
cmake -B build
cmake --build build
```

可选向脚本传入额外 CMake 参数（会传给 Lua、GTest、Protobuf、gRPC、gflags 五处 `cmake -S`），例如：

```bash
./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Release
```

若 `library/` 中缺少 Lua 或 GTest，主工程 `cmake` 会 **FATAL_ERROR** 并提示运行上述脚本；缺少 **Protobuf** / **gRPC** / **gflags** 时对应 **`find_package(REQUIRED)`** 会失败，需先运行安装脚本（或自行安装到 **`library/protobuf/`**、**`library/grpc/`**、**`library/gflags/`**）。

**故障排除**：Lua 路径类错误多为旧缓存：删除 `build/` 后重新 `cmake -B build`。误装到 **`/usr/local/googletest`** 时可：`sudo rm -rf /usr/local/googletest`。

### 3. 与 evrp-device 相关依赖

- **Protobuf**（含 **`protoc`**）安装到 **`library/protobuf/`**；**gRPC C++**（含 **`grpc_cpp_plugin`**）安装到 **`library/grpc/`**；**gflags** 安装到 **`library/gflags/`**（或由安装脚本写入）后，再运行本仓库 `cmake` 配置。

- 自行从源码构建时，示例：

  ```bash
  cmake -S <grpc-source> -B grpc-build -DCMAKE_INSTALL_PREFIX="$PWD/library"
  cmake --build grpc-build --target install
  ```

  （具体工程与目标名以对应上游文档为准。）

- `library/` 目录说明：[`../LIBRARY.md`](../LIBRARY.md)

## C++ 命名（本仓库业务代码）

- **函数、方法、自由函数**：`snake_case`（例如 `run_device_server`、`read_input_events`）。
- **类型**（`class` / `struct` / `enum` / `using`）：`PascalCase`（例如 `IInputListener`、`ApiResult`）。
- **与 gRPC / protobuf 生成代码的接口**：保持生成器产生的名称（如 `InputListenService::ReadInputEvents`），适配层再转为 `api` 层类型与命名。
