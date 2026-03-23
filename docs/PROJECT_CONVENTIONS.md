# 项目规范

## 第三方依赖与目录约定

### 1. `library/` — 第三方**安装根目录**

- 所有**非本仓库源码自带**、需单独编译或预编译安装的第三方库，**统一安装到仓库根目录下的 `library/`**。

- 约定布局：

  | 子目录 | 用途 |
  |--------|------|
  | `library/bin` | 可执行工具（如 `protoc`、`grpc_cpp_plugin`） |
  | `library/include` | 第三方头文件 |
  | `library/lib` | 库文件及 CMake 的 `*.cmake` 包配置 |
  | `library/share` | 按需（部分工具链使用） |

- 顶层 **`CMakeLists.txt`** 定义 **`EVRP_LIBRARY_DIR`**（`<源码根>/library`），Lua/GTest 及 evrp-device 中的 `find_program` 等据此查找。若要让 **`find_package(Protobuf)` / `find_package(gRPC)`** 使用 `library/` 中的安装，请在配置时传入 **`-DCMAKE_PREFIX_PATH=<repo>/library`**（或设置环境变量 `CMAKE_PREFIX_PATH`）。

- 安装到 `library/` 的**具体文件**不纳入版本控制（见根目录 `.gitignore`）；仅保留 `library/README.md` 说明用途。

### 2. Lua / GoogleTest：主工程**仅从 `library/` 获取**

- 根 **`CMakeLists.txt`** 只在 **`library/`** 下查找 Lua 与 GoogleTest（`NO_DEFAULT_PATH`），不编译 `third_party` 源码。
- 配置主工程 **前**，须已把 Lua / GTest 安装到 **`library/lua/`** 与 **`library/googletest/`**（或兼容的旧布局，仍限于 `library/` 树内）。
- **`third_party/lua`、`third_party/googletest`** 仅保留源码；**编译与安装**由 **`scripts/install-third-party-to-library.sh`** 完成（脚本内直接 `cmake -S third_party/...`，不依赖仓库内 `cmake/` 模块）。

**首次或更新第三方源码后：**

```bash
./scripts/install-third-party-to-library.sh
cmake -B build
cmake --build build
```

可选向脚本传入额外 CMake 参数（会传给 Lua 与 GTest 两处配置），例如：

```bash
./scripts/install-third-party-to-library.sh -DCMAKE_BUILD_TYPE=Release
```

若 `library/` 中缺少 Lua 或 GTest，主工程 `cmake` 会 **FATAL_ERROR** 并提示运行上述脚本。

**故障排除**：Lua 路径类错误多为旧缓存：删除 `build/` 后重新 `cmake -B build`。误装到 **`/usr/local/googletest`** 时可：`sudo rm -rf /usr/local/googletest`。

### 3. 与 evrp-device 相关依赖

- **Protobuf、gRPC C++、gflags** 及 `protoc` / `grpc_cpp_plugin` 应安装到 **`library/`**，再运行本仓库 `cmake` 配置。

- 自行从源码构建时，示例：

  ```bash
  cmake -S <grpc-source> -B grpc-build -DCMAKE_INSTALL_PREFIX="$PWD/library"
  cmake --build grpc-build --target install
  ```

  （具体工程与目标名以对应上游文档为准。）

- `library/` 目录说明：[`../library/README.md`](../library/README.md)
