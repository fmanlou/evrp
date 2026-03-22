# evrp Protobuf / gRPC 定义

## 职责

**evrp-device** 通过 `InputDeviceService` 提供：

| RPC | 说明 |
|-----|------|
| `ReadInputEvents` | 实时读输入 → stream `InputEvent` |
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
