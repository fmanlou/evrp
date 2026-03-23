# evrp-device

设备端进程：入口 **`main.cpp` 仅** 包含 `evrp/device/api/server.h` 与 **`IDeviceHost`** 实现（如 `StubDeviceHost`），**不包含** gRPC 头文件。

传输与 proto 全部在 **`evrp/device/internal/*.cpp`** 与 CMake 目标 **`evrp_device_grpc`** 中实现（见 [`docs/API_LAYER.md`](../docs/API_LAYER.md)）。

## 依赖（构建）

- CMake ≥ 3.14
- C++17
- [gRPC](https://grpc.io/) C++、`protobuf`、`protoc`、`grpc_cpp_plugin`（建议 **`library/grpc/`**、**`library/protobuf/`**）  
- [gflags](https://github.com/gflags/gflags)（命令行解析；建议 **`library/gflags/`**）

推荐执行 [`scripts/install-third-party-to-library.sh`](../scripts/install-third-party-to-library.sh) 将 Lua、GoogleTest、Protobuf、gRPC、gflags 等安装到 [`library/`](../LIBRARY.md)（见 [`docs/PROJECT_CONVENTIONS.md`](../docs/PROJECT_CONVENTIONS.md)）。亦可用系统包，例如 Debian/Ubuntu：`libgrpc++-dev` `libprotobuf-dev` `protobuf-compiler-grpc` `libgflags-dev`。

## 构建

在仓库根目录安装上述依赖后，**默认**会生成 **`evrp-device`**（与 `evrp` 同一次配置）：

```bash
cmake -B build
cmake --build build
# 或：cmake --build build --target evrp-device
```

生成的可执行文件：`build/evrp-device`（或构建目录下的目标名）。

## 运行

```bash
./build/evrp-device -listen=127.0.0.1:50051
# 或 --listen=... ；查看帮助：./build/evrp-device -help
```

当前首版仅 **`Ping`** 返回成功，其余 RPC 为 **UNIMPLEMENTED**（见 `docs/EVRP_DEVICE_DEVELOPMENT_PLAN.md`）。
