# evrp Protobuf / gRPC 定义

## 职责

业务代码通过 **`evrp/device/{api,internal,server}/`**（如 `api/server.h` 等）访问设备端能力，**不** include 本目录生成的 `*.pb.h` 或 gRPC（proto 互转见 `internal/deviceprotoconv.h`）。

**evrp-device** 通过 gRPC `InputDeviceService` 提供：

| RPC | 说明 |
|-----|------|
| `StartRecording` | 指定 `DeviceKind` 并开始采集（开始录制）→ `Empty` |
| `ReadInputEvents` | 仅拉取事件流 → `stream InputEvent`（宜在 `StartRecording` 之后） |
| `StopRecording` | 停止录制 → `Empty` |
| `UploadRecording` | `UploadRecordingFrame`：开始帧 → 中间帧（`data` + `checksum`）→ 结束帧；下行 `UploadRecordingStatus`（`code` / `message`） |
| `PlaybackRecording` / `StopPlayback` | 回放当前已缓存资源；`StopPlayback` 返回 `Empty` |
| `GetCursorPositionAvailability` | 查询读光标坐标是否可用（响应仅 `available`） |
| `ReadCursorPosition` | 当前光标屏幕坐标（`x` / `y`，像素） |
| `Ping` | 保活 |

**evrp-app**：录制、落盘、Lua、发起上传与回放。

## 布局

- `evrp/device/v1/device.proto` — `InputDeviceService`

## 生成 C++ 代码（示例）

```bash
protoc -I proto \
  --cpp_out=generated/cpp \
  --grpc_out=generated/cpp \
  --plugin=protoc-gen-grpc="$(which grpc_cpp_plugin)" \
  proto/evrp/device/v1/device.proto
```

## 版本

**v1**（`package evrp.device.v1`）。
