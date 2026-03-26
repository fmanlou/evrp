# 设备 API 抽象层（业务不可见 gRPC / proto）

业务代码按角色 include **`evrp/device/api/`**（`IInputListener` 与输入相关 **`api/types.h`**）、**`internal/`**（扩展值类型、`deviceprotoconv`）、**`server/`**（设备端主机与启动服务），**不** `#include` 任何 `*.pb.h`、`grpcpp` 或 `InputDeviceService::Stub`（`internal/deviceprotoconv.h` 除外，供已与 gRPC 同库的适配层使用）。

## 公共头文件（无 gRPC，除 deviceprotoconv）

| 头文件 | 说明 |
|--------|------|
| `evrp/device/api/types.h` | **`DeviceKind`**、**`InputEvent`**（`IInputListener` 等使用） |
| `evrp/device/api/inputlistener.h` | **`IInputListener`**：进程内输入监听会话（非 gRPC） |
| `evrp/device/internal/types.h` | 其余值类型与 `ApiResult` / `ApiError`（依赖 `api/types.h`） |
| `evrp/device/internal/deviceprotoconv.h` | `api` 类型 ↔ `device.proto`（依赖 protobuf，见 CMake **`deviceprotoconv`**） |
| `evrp/device/server/host.h` | **`IDeviceHost`**：设备端能力 |
| `evrp/device/server/server.h` | **`run_device_server(address, IInputListener&)`**：启动服务并阻塞；录制相关 RPC 使用 `IInputListener` |

## gRPC / proto 封装位置（实现细节）

仅以下翻译单元包含 gRPC 与生成代码，**勿**在业务源码中 include：

| 路径 | 说明 |
|------|------|
| `evrp/device/server/grpcserverimpl.cpp` | `run_device_server`：ServerBuilder、监听 |
| `evrp/device/server/grpcinputdeviceservice.*` | `InputDeviceService::Service`：录制相关 RPC ↔ `IInputListener`，其余 ↔ `IDeviceHost` |

CMake 目标 **`evrp_device_grpc`**（静态库）聚合上述实现；可执行文件 **`evrp-device`** 链接该库即可，**无需**再直接链接 proto 目标（除非另有工具链需求）。业务侧若需作为 gRPC **客户端** 连接设备，需在应用代码中自行使用生成 stub 或单独封装（本仓库不再提供 `evrp/device/client`）。

## 依赖方向

```
业务 / main.cpp / 设备核心
    → 按需 include evrp/device/api|internal|server 下头文件
    → 进程内输入监听见 `evrp/device/server/localinputlistener.h`（`evrp::device::server::LocalInputListener` 实现 `IInputListener`）
    → 实现 IDeviceHost 时不包含 grpc

evrp_device_grpc（内部 .cpp）
    → grpc++、device.grpc.pb.h（不暴露给上层）
```
