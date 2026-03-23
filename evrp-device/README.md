# evrp-device

设备端 gRPC 服务，实现 `proto/evrp/device/v1/device.proto` 中的 `InputDeviceService`。

## 依赖（构建）

- CMake ≥ 3.14
- C++11
- [gRPC](https://grpc.io/) C++、`protobuf`、`protoc`、`grpc_cpp_plugin`  
  例如 Debian/Ubuntu：`libgrpc++-dev` `libprotobuf-dev` `protobuf-compiler-grpc`

## 构建

在仓库根目录（**必须**打开 `EVRP_BUILD_DEVICE`，并安装上述依赖）：

```bash
cmake -B build -DEVRP_BUILD_DEVICE=ON
cmake --build build --target evrp-device
```

未安装 gRPC 时，不要开启该选项，默认的 `evrp` 主目标仍可正常配置。

生成的可执行文件：`build/evrp-device`（或构建目录下的目标名）。

## 运行

```bash
./build/evrp-device --listen 127.0.0.1:50051
```

当前首版仅 **`Ping`** 返回成功，其余 RPC 为 **UNIMPLEMENTED**（见 `docs/EVRP_DEVICE_DEVELOPMENT_PLAN.md`）。
