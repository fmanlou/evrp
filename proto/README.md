# evrp Protobuf / gRPC 定义

## 职责

业务代码通过 **`evrp/device/{api,internal,server}/`**（如 `api/server.h` 等）访问设备端能力，**不** include 本目录生成的 `*.pb.h` 或 gRPC（proto 互转见 `internal/tofromproto.h`：`ToProto` / `FromProto`）。

**evrp-device** 在同一监听端口注册 **三个** gRPC 服务。传输层连接存活由 **gRPC HTTP/2 keepalive** 配置（客户端 `makeDeviceChannel`、服务端 `runDeviceServer`）。

### `InputListenService`（`service/inputlisten.proto`）

Unary 输入监听（**`device_client` 远程实现**走本服务）。

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
| `Playback` | 在 device 上回放当前已缓存资源 → `OperationResult`；宜与 `SubscribePlayback` 并行（先订阅） |
| `SubscribePlayback` | 服务端流：`PlaybackProgress`（`done=false` 为当前事件；`done=true` + `result` 为整场结束，与 Unary `Playback` 一致） |
| `Stop` | 停止回放 → `Empty` |

### `InputDeviceService`（`service/service.proto`）

| RPC | 说明 |
|-----|------|
| `GetCursorPositionAvailability` | 查询读光标坐标是否可用（响应仅 `available`） |
| `ReadCursorPosition` | 当前光标屏幕坐标（`x` / `y`，像素） |
| `GetCapabilities` | 当前可识别的输入设备类型列表 → `GetCapabilitiesResponse.supported_kinds`（无低层 evdev 能力位） |

**evrp-app**：录制、落盘、Lua、发起上传与回放。

## 布局

- `evrp/device/v1/types/common.proto` — 跨服务类型（`DeviceKind`、`InputEvent`）
- `evrp/device/v1/types/{inputlisten,playback,service}.proto` — 与同名 `service/*.proto` 一一对应的消息
- `evrp/device/v1/service/{inputlisten,playback,service}.proto` — 各 gRPC `service` 定义

（每个 `service/*.proto` 只 `import` 与之同名的 `types/<name>.proto`；后者按需再 `import` `types/common.proto`。）

## 生成 C++ 代码（示例）

```bash
protoc -I proto \
  --cpp_out=generated/cpp \
  --grpc_out=generated/cpp \
  --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
  proto/evrp/device/v1/types/common.proto \
  proto/evrp/device/v1/types/inputlisten.proto \
  proto/evrp/device/v1/types/playback.proto \
  proto/evrp/device/v1/types/service.proto \
  proto/evrp/device/v1/service/inputlisten.proto \
  proto/evrp/device/v1/service/playback.proto \
  proto/evrp/device/v1/service/service.proto
```

## 版本

**v1**（`package evrp.device.v1`）。
