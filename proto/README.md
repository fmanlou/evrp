# evrp Protobuf / gRPC 定义

## 职责

业务代码通过 **`evrp/device/{api,internal,server}/`**（如 `api/server.h` 等）访问设备端能力，**不** include 本目录生成的 `*.pb.h` 或 gRPC（proto 互转见 `internal/tofromproto.h`：`ToProto` / `FromProto`）。

**evrp-device** 在同一监听端口注册 **三个** gRPC 服务：

### `InputListenService`（`service/inputlisten.proto`）

| RPC | 说明 |
|-----|------|
| `StartRecording` | 指定 `DeviceKind` 并开始采集（开始录制）→ `Empty` |
| `WaitForInputEvent` | `timeout_ms`，对应 `wait_for_input_event` → `WaitForInputEventResponse`（`ready`） |
| `ReadInputEvents` | 一次拉取就绪批次 → `ReadInputEventsResponse`（`repeated InputEvent`） |
| `StopRecording` | 停止录制 → `Empty` |

### `PlaybackService`（`service/playback.proto`）

| RPC | 说明 |
|-----|------|
| `Upload` | Unary：一次 `UploadRecordingRequest`（`repeated InputEvent`），应答 `OperationResult`（`code` / `message`） |
| `Playback` | 在 device 上回放当前已缓存资源 → `OperationResult` |
| `Stop` | 停止回放 → `Empty` |

### `InputDeviceService`（`service/service.proto`）

| RPC | 说明 |
|-----|------|
| `GetCursorPositionAvailability` | 查询读光标坐标是否可用（响应仅 `available`） |
| `ReadCursorPosition` | 当前光标屏幕坐标（`x` / `y`，像素） |
| `Ping` | 保活 |

**evrp-app**：录制、落盘、Lua、发起上传与回放。

## 布局

- `evrp/device/v1/types.proto` — v1 消息与枚举（无 `service`）
- `evrp/device/v1/service/inputlisten.proto` — `InputListenService`
- `evrp/device/v1/service/playback.proto` — `PlaybackService`
- `evrp/device/v1/service/service.proto` — `InputDeviceService`

（上述 service 文件均 `import` `evrp/device/v1/types.proto`。）

## 生成 C++ 代码（示例）

```bash
protoc -I proto \
  --cpp_out=generated/cpp \
  --grpc_out=generated/cpp \
  --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
  proto/evrp/device/v1/types.proto \
  proto/evrp/device/v1/service/inputlisten.proto \
  proto/evrp/device/v1/service/playback.proto \
  proto/evrp/device/v1/service/service.proto
```

## 版本

**v1**（`package evrp.device.v1`）。
