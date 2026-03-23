# 设备 API 抽象层（业务不可见 gRPC / proto）

业务代码（evrp-app、设备核心实现、**evrp-device 的 main**）只通过 **`evrp/device/api/`** 下的头文件交互，**不** `#include` 任何 `*.pb.h`、`grpcpp` 或 `InputDeviceService::Stub`。

## 业务可见的公共头文件（无 gRPC）

| 头文件 | 说明 |
|--------|------|
| `evrp/device/api/types.h` | 值类型与 `ApiResult` |
| `evrp/device/api/host.h` | **`IDeviceHost`**：设备端能力 |
| `evrp/device/api/client.h` | **`IDeviceClient`**：业务侧调用设备 |
| `evrp/device/api/server.h` | **`RunDeviceServer(address, IDeviceHost&)`**：启动服务并阻塞 |
| `evrp/device/api/client_factory.h` | **`ConnectDeviceClient(target)`**：返回 `IDeviceClient` |

## gRPC / proto 封装位置（实现细节）

仅以下翻译单元包含 gRPC 与生成代码，**勿**在业务源码中 include：

| 路径 | 说明 |
|------|------|
| `evrp/device/internal/grpc_server_impl.cpp` | `RunDeviceServer`：ServerBuilder、监听 |
| `evrp/device/internal/grpc_input_device_service.*` | `InputDeviceService::Service` ↔ `IDeviceHost` |
| `evrp/device/internal/grpc_device_client.cpp` | gRPC stub ↔ `IDeviceClient` |

CMake 目标 **`evrp_device_grpc`**（静态库）聚合上述实现；可执行文件 **`evrp-device`** 与未来的 **`evrp-app`** 链接该库即可，**无需**再直接链接 proto 目标（除非另有工具链需求）。

## 依赖方向

```
业务 / main.cpp / 设备核心
    → 仅 include evrp/device/api/*.h 与 evrp/device/stub_device_host.h（桩）
    → 实现 IDeviceHost 时不包含 grpc

evrp_device_grpc（内部 .cpp）
    → grpc++、device.grpc.pb.h（不暴露给上层）
```
