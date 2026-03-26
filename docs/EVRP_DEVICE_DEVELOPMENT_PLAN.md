# evrp-device 开发方案

本文档在 [ARCHITECTURE_SPLIT_PLAN.md](./ARCHITECTURE_SPLIT_PLAN.md) 与 `proto/evrp/device/v1/device.proto` 基础上，给出 **evrp-device**（设备端 gRPC 服务）的落地开发方案。

---

## 1. 目标与边界

| 项目 | 说明 |
|------|------|
| **目标** | 独立进程，在具备 `/dev/input/*`（及可选 X11）的机器上运行，对外仅通过 **`InputDeviceService`** 暴露能力 |
| **协议来源** | 以仓库内 **`.proto` 为唯一契约**；实现侧不发明与 proto 冲突的语义 |
| **非目标（首版可不做）** | 多租户隔离、细粒度配额、录制资源的列举/删除 RPC（由实现内部策略处理） |

---

## 2. 与现有代码的映射

设备能力应先实现 **`evrp::device::api::IDeviceHost`**（见 [`API_LAYER.md`](./API_LAYER.md)）；启动服务用 **`api::run_device_server`**，**不要**在业务或 `main` 中 include `grpcpp` / `*.pb.h`。业务侧作为 gRPC 客户端连接设备时，使用本仓库生成的 stub 或自行封装（本仓库不再提供独立设备客户端库）。

当前单体程序中的模块可复用到 evrp-device：

| 能力 | 现有代码（参考） | evrp-device 中的职责 |
|------|------------------|----------------------|
| 发现/打开 evdev、`read` 事件 | `evdev.*`、`inputdevice.*`、`touchdevice.*`、`keyboard/*` | `StartRecording` 按 `DeviceKind` 选择设备并启动采集；读线程将事件转为 `InputEvent` |
| 注入回放 | `inputeventwriter.*`、`mouse/*`、`keyboard/*` | `PlaybackRecording` 读**当前缓存**的 eventformat（或等价字节流），经 `InputEventWriter` 注入 |
| 光标 | `cursor/cursorpos.*` | `GetCursorPositionAvailability` / `ReadCursorPosition` 直接委托 `CursorPos` |
| 文件/缓冲 | `filesystem.*`（可选） | `UploadRecording` 拼接帧 → 落盘或内存，作为「当前可回放」资源 |

**不复用进 device 的**：`record.*` / `playback.*` 的上层流程、Lua、`eventformat` 的**业务侧**编排（属 evrp-app）。

---

## 3. 运行时模型（建议）

### 3.1 读路径：`StartRecording` → `ReadInputEvents` → `StopRecording`

- **会话模型（建议首版）**：单连接内 **一次**「读会话」：先 `StartRecording`（携带 `kinds`），再 **一个** `ReadInputEvents` 服务端流；`StopRecording` 结束采集并结束/取消流。
- **并发**：采集线程与 gRPC 写流解耦：环形缓冲或阻塞队列，**`ReadInputEvents`** 协程/线程从队列取事件 `Write` 到流。
- **背压**：队列满时的策略（丢事件计数 + `UploadRecordingStatus` 式告警 **不**适用于读流；读流建议阻塞或合并策略，首版可 **阻塞** 并文档说明延迟风险）。
- **`DeviceKind` → 设备路径**：沿用 `find_first_*` / `find_device_path` 与 `DeviceId` 映射；`UNSPECIFIED` 的语义在实现里固定（例如忽略或报错）。

### 3.2 上传与回放

- **上传**：严格帧序 **start → N × middle（校验 `checksum`）→ end**；拼接后写入 **单一「当前资源」** 槽（路径或内存句柄）。
- **下行 `UploadRecordingStatus`**：实现可周期性 `bytes_received` 等价逻辑已通过删除字段简化为仅 `code`/`message`；至少 **最后一帧** 表示成功或失败。
- **回放**：`PlaybackRecording` 无参，只读 **最近一次成功上传** 的资源；与 **读会话** 互斥策略需在实现中定义（首版建议：**回放时拒绝 `StartRecording`** 或 **停止读后再回放**）。

### 3.3 光标

- `GetCursorPositionAvailability`：`CursorPos::is_available()`（或等价）。
- `ReadCursorPosition`：`get_position`；若不可用，行为由实现约定（首版：**先查 availability**，否则返回 `UNIMPLEMENTED` 或 `(0,0)` —— 需在实现 README 中写死一种）。

### 3.4 保活

- `Ping`：空实现返回默认 `PingResponse` 即可（若 `PingResponse` 无字段则空消息）。

---

## 4. 工程与构建

| 项 | 建议 |
|----|------|
| **生成代码** | `protoc` + `grpc_cpp_plugin` 生成 `evrp/device/v1/*.pb.{h,cc}` 与 `*.grpc.pb.{h,cc}`；纳入 CMake 目标 `evrp_device_proto` |
| **依赖** | `protobuf`、`grpc++`、`gflags`（`evrp-device` 入口）、pthread；建议安装到 [`library/`](../library/)（见 [`PROJECT_CONVENTIONS.md`](./PROJECT_CONVENTIONS.md)）；可选链接 X11（与现有 `cursorposx11` 一致） |
| **产物** | 可执行文件 **`evrp-device`**：默认与 `evrp` 同次配置构建，详见 [`evrp-device/README.md`](../evrp-device/README.md) |
| **监听** | 首版 **Unix Domain Socket**（如 `/run/user/.../evrp.sock`）或 `127.0.0.1:端口`；地址通过 **命令行或环境变量** 配置 |

---

## 5. 分阶段交付（里程碑）

### M0：骨架

- [ ] CMake 集成 gRPC/protobuf，生成并编译 proto
- [ ] `InputDeviceService` 空实现 + `Ping` 打通
- [ ] 命令行：监听地址、日志级别、help

### M1：读事件路径（核心）

- [ ] `StartRecording`：解析 `kinds`，打开对应 evdev，启动读线程
- [ ] `ReadInputEvents`：推送 `InputEvent`（与内核 `input_event` + `DeviceKind` 对齐）
- [ ] `StopRecording`：停止线程、关闭 fd、结束流
- [ ] 单测或集成测：mock evdev 或 `/dev/input` 可用环境

### M2：上传 + 回放

- [ ] `UploadRecording`：帧解析、CRC 校验、落盘/缓存
- [ ] `PlaybackRecording`：读缓存、解析 eventformat、走 `InputEventWriter`
- [ ] `StopPlayback`：停止注入循环（与读路径互斥策略）

### M3：光标

- [ ] `GetCursorPositionAvailability` / `ReadCursorPosition` 对接 `CursorPos`

### M4：硬化

- [ ] 错误与日志规范；`max_send`/`max_recv` 与上传分块大小建议
- [ ] 文档：`docs/` 或 `README` 片段「部署与权限（input 组、X11 `DISPLAY`）」
- [ ] （可选）SIGTERM 优雅退出、uds 权限 mask

---

## 6. 测试策略

| 类型 | 内容 |
|------|------|
| **单元** | 帧拼接与 CRC、eventformat 解析（若抽成库） |
| **集成** | gRPC 客户端（最小 stub）连本机 `evrp-device`，覆盖 M1～M3 主路径 |
| **手动** | 真机 `/dev/input` + X11 下回放与光标 |

---

## 7. 风险与依赖

- **权限**：用户须在 `input` 组或有权限读 `/dev/input`；注入可能需 uinput（若当前实现已用，需一并文档化）。
- **HEAD / 多客户端**：首版可 **单客户端单会话**；多连接并发需在后续版本明确排队或拒绝策略。
- **与 evrp-app 的联调**：以 proto 冻结为前提，先 **同机 UDS** 联调，再考虑 TCP/TLS。

---

## 8. 文档维护

- 协议变更：**先改 `.proto` 与 `proto/README.md`**，再同步本方案与 `ARCHITECTURE_SPLIT_PLAN.md`。
- 实现细节（缓存路径、队列长度、互斥规则）放在 **代码旁 README 或 `docs/` 实现说明**，避免写死在架构文档页。
