# 设备 API 抽象层（不暴露 proto / gRPC）

业务与设备核心逻辑**不**直接依赖 `device.proto` 或 `grpc::Service`，而是通过 **`evrp/device/api/`** 中的类型与接口交互。

## 布局

| 路径 | 说明 |
|------|------|
| `evrp/device/api/types.h` | 与 proto 对齐的值类型（`DeviceKind`、`InputEvent`、`RecordingStatus` 等）及 `ApiError` / `ApiResult` |
| `evrp/device/api/host.h` | **`IDeviceHost`**：设备端能力（读事件、上传、回放、光标等） |
| `evrp/device/api/client.h` | **`IDeviceClient`**：业务端（evrp-app）侧对称接口，便于 Mock / 非 gRPC 实现 |
| `evrp/device/stub_device_host.*` | 默认桩实现（当前仅 `Ping` 成功） |
| `evrp-device/grpc/grpc_input_device_service.*` | **仅此**将 `InputDeviceService` gRPC 映射到 `IDeviceHost`（proto ↔ api 转换） |

## 依赖方向

```
业务 / 设备核心  →  include api/*.h（仅 C++ 类型）
       ↑ 实现
IDeviceHost 实现（evdev、回放注入等）

gRPC 适配器（GrpcInputDeviceService）→ 实现 grpc::Service，内部调 IDeviceHost
       ↑ 链接
evrp-device 可执行文件：StubDeviceHost + GrpcInputDeviceService
```

## evrp-app

录制/回放/Lua 等业务应依赖 **`IDeviceClient`**（或封装后的 Facade），由 **`GrpcDeviceClient`**（后续实现）在内部使用 gRPC stub，**不**在业务代码中 include `*.pb.h`。
