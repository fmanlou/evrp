# evrp 架构分离方案

## 目标

将 evrp 拆分为两个独立程序：

1. **设备端程序 (evrp-device)**：运行在配备物理输入设备的机器上，负责输入事件的采集与注入
2. **业务端程序 (evrp-app)**：与设备无关，可运行在任何环境，负责业务逻辑处理

---

## 一、职责划分

### 1.1 设备端程序 (evrp-device)

**运行环境**：必须能访问 `/dev/input/*` 的机器（通常为被测设备或测试台）

**职责（仅底层实现）**：通过 `InputDeviceService` 提供 **实时读**、**录制资源缓存**、**回放当前缓存资源注入**与 **光标坐标查询**。
- **读**：**`StartRecording`** 指定设备种类并开始采集；**`ReadInputEvents`** 仅建立 **事件流**（`stream InputEvent`）；**`StopRecording`** 停止。
- **缓存与回放**：接收 app **完整推送**的录制资源（`UploadRecording` 流式上传字节），落盘或内存缓存；**`PlaybackRecording`** 无参，回放 **当前** 已缓存资源（本地解析并 `InputEventWriter` 注入）。**不提供** app 侧业务语义（落盘路径、Lua、编排仍属 app）。
- **光标**：先 **`GetCursorPositionAvailability`** 查询读坐标是否可用（响应 **`available`**）；再 **`ReadCursorPosition`** 取 **屏幕像素坐标**（响应仅 **`x` / `y`**）。不可用时不将「无显示」作为 **`ReadCursorPosition`** 的 gRPC 错误——由调用方先查可用性。

**依赖**：
- Linux input 子系统 (`/dev/input/event*`)
- 可选：X11 / 显示后端（`GetCursorPositionAvailability` / `ReadCursorPosition`、注入时若需 `cursorpos`）
- gRPC 服务端；录制资源缓存（策略见 evrp-device 实现）

**不承担**：
- 录制、回放流程控制与文件输出
- 事件格式转换、Lua、业务编排（由 evrp-app 完成）

---

### 1.2 业务端程序 (evrp-app)

**运行环境**：任意环境（本地、服务器、CI、容器等），无需物理输入设备

**职责（含录制与回放控制）**：
- **录制**：先 **`StartRecording`**，再 **`ReadInputEvents`** 消费流，将 `InputEvent` 转为 eventformat、本地落盘与策略；需要 device 回放时，将**完整录制文件**经 **`UploadRecording`** 推送到 device
- **回放**：调用 **`PlaybackRecording()`**（无参），由 device 使用**当前**已缓存资源注入；**不再**使用流式逐条 `WriteInputBatch`，避免回放与 RPC 时序耦合
- 事件文件生成、编辑、Lua、解析、存储、分析等

**依赖**：
- 文件系统
- gRPC 客户端（连接 evrp-device）
- Lua 解释器
- 不依赖 `/dev/input/*`、X11 等设备接口（坐标类需求可通过协议要求 device 侧处理或返回数据）

---

## 二、通信架构

**协议与接口统一采用 [gRPC](https://grpc.io/)（C++ 使用 `grpc` / `grpc++` 与 `protobuf`）**：IDL 用 `.proto` 定义服务、RPC 与消息类型；传输层由 gRPC 处理（HTTP/2，本地可用 Unix Domain Socket 或 `localhost` TCP）。

**`.proto` 定义位置**：仓库内 `proto/evrp/device/v1/device.proto`（`InputDeviceService`），详见同目录 `proto/README.md`。

**第三方依赖目录约定**（`library/` 与 `third_party/`）：见 [`docs/PROJECT_CONVENTIONS.md`](./PROJECT_CONVENTIONS.md)。

```
┌─────────────────────────────────────────────────────────────────┐
│                      evrp-app (业务端)                            │
│  - 录制 / 回放控制与落盘                                          │
│  - eventformat、Lua、编排、存储                                   │
│  - gRPC Client                                                    │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                    gRPC (HTTP/2)
                            │
┌───────────────────────────▼─────────────────────────────────────┐
│                    evrp-device (设备端)                           │
│  - gRPC Server：`InputDeviceService`                              │
│  - StartRecording / ReadInputEvents: 开始采集 + 事件流 → app        │
│  - UploadRecording: app 推送完整资源 → 缓存                         │
│  - PlaybackRecording(): 回放当前缓存、本地注入                       │
│  - GetCursorPositionAvailability / ReadCursorPosition: 可用性 + 坐标 → app │
└─────────────────────────────────────────────────────────────────┘
```

### 2.1 接口形态（以 `proto/evrp/device/v1/device.proto` 为准）

| 能力 | 归属 | gRPC 形态 |
|------|------|-----------|
| 开始读、事件流、停止读 | device | `StartRecording(StartRecordingRequest)` → `Empty`；`ReadInputEvents(google.protobuf.Empty)` → `stream InputEvent`；`StopRecording(Empty)` → `Empty` |
| 上传完整录制资源 | device | `UploadRecording(stream UploadRecordingFrame)`：开始/中间/结束帧 → `stream UploadRecordingStatus`（`code` / `message`） |
| 回放当前缓存 | device | `PlaybackRecording` Unary（`PlaybackRecordingRequest` 空） |
| 停止回放 | device | `StopPlayback(google.protobuf.Empty)` → `google.protobuf.Empty` |
| 读光标是否可用 | device | `GetCursorPositionAvailability` Unary → `GetCursorPositionAvailabilityResponse`（`available`） |
| 读光标屏幕坐标 | device | `ReadCursorPosition` Unary → `ReadCursorPositionResponse`（`x` / `y`） |
| 保活 | device | `Ping` Unary（空消息） |
| 录制业务、文件主编排 | **evrp-app** | 消费 `InputEvent`、生成完整资源文件、再 `UploadRecording` |
| 发起回放 | **evrp-app** | `PlaybackRecording()` 无参，**无** WriteBatch 逐条注入 |

### 2.2 载荷说明

- **实时读**：先 **`StartRecording`**（`kinds`），再 **`ReadInputEvents`** 拉取 **`InputEvent`** 流；停止用 **`StopRecording`**。
- **上传资源**：客户端先发 **开始帧**，再若干 **中间帧（数据帧）**（`middle.data`，每帧 **`middle.checksum`** 校验本帧数据），拼接为完整资源（与现有 eventformat 文本文件一致），最后 **结束帧**；device 持久化或缓存后通过 **`UploadRecordingStatus`** 流回馈，下行最后一帧为 **`code` / `message`**（避免大包 unary 与长时间无下行）。
- **回放**：device 读取**当前**本地缓存（通常为最近一次成功上传），解析并注入；**回放时序与 RPC 推送解耦**，无逐条 Write 延迟问题。
- **光标**：**`GetCursorPositionAvailability`** 返回是否可读；**`ReadCursorPosition`** 仅返回坐标。是否可用**不**放在 `ReadCursorPositionResponse` 中。
- 上传流注意 `max_send_message_size`。

---

## 三、模块归属

| 模块 | 设备端 (evrp-device) | 业务端 (evrp-app) |
|------|----------------------|-------------------|
| Record（流程、落盘、格式） | ✗ | ✓ |
| Playback（流程、读文件、节奏） | ✗ | ✓ |
| InputDevice / evdev / 读事件 | ✓ | ✗ |
| InputEventWriter（注入实现） | ✓ | ✗ |
| CursorPos (X11) | ✓（若需要） | ✗ |
| eventformat (解析/生成) | ✓（`PlaybackRecording` 读缓存文件并解析注入） | ✓（录制、组装完整文件再 `UploadRecording`） |
| Lua + lua_bindings | ✗ | ✓ |
| argparser | 部分 | 部分 |
| logger, filesystem | ✓ | ✓ |

**Lua**：仅在 **evrp-app** 执行；生成完整资源文件后 **`UploadRecording`**，再 **`PlaybackRecording()`**。

---

## 四、典型工作流

### 4.1 录制流程（业务在 app）

```
evrp-app                    evrp-device
    |                            |
    |--- StartRecording --------->| 按 kinds 打开设备并开始采集
    |--- ReadInputEvents -------->| 建立事件流（Empty）
    |<--- InputEvent (stream) ---| 输入事件
    |                            |
    |--- StopRecording ---------->| 停止读（可选）
    |  app 侧：转为 eventformat、写文件、策略控制   |
```

### 4.2 回放流程（资源缓存 + 无参回放）

```
evrp-app                         evrp-device
    |                                 |
    | 准备完整录制文件（本地/eventformat/Lua 导出） |
    |--- UploadRecording (开始/中间/结束帧 + status stream) --->| 拼接并缓存当前可回放资源
    |<--- UploadRecordingStatus（最终帧 code/message）--------|
    |                                 |
    |--- PlaybackRecording() --------->| 读当前缓存、解析、注入设备
    |<--- PlaybackRecordingResponse --| 回放结束
    |                                 |
```

### 4.3 纯业务处理（无设备）

```
evrp-app
    |
    | 解析事件文件
    | 执行 Lua（生成、转换）
    | 写入新文件 / 分析 / 编排
```

---

## 五、实施阶段建议

**evrp-device 细化开发步骤（里程碑、运行时模型、测试）见 [EVRP_DEVICE_DEVELOPMENT_PLAN.md](./EVRP_DEVICE_DEVELOPMENT_PLAN.md)。**

### 阶段一：协议与接口设计（gRPC）
- 编写 `.proto`：服务名、RPC 列表、请求/响应消息、流式语义（Unary / streaming）
- 选用 `grpc_cpp_plugin` 生成 C++ stub 与 service 基类；CMake 集成 `find_package(gRPC)` 或 `FetchContent`/`add_subdirectory` 引入依赖
- 约定监听地址（如 `unix:///path/evrp.sock` 或 `host:port`）、TLS（若跨机）与消息大小限制

### 阶段二：evrp-device 抽取
- 将 **InputDevice、evdev、InputEventWriter**、**录制资源存储**（及 gRPC 适配层）打包为独立可执行程序
- 实现 gRPC **服务端**：`StartRecording`、`ReadInputEvents`、`UploadRecording`、`PlaybackRecording`、`GetCursorPositionAvailability`、`ReadCursorPosition` 等

### 阶段三：evrp-app 抽取
- 将 **Record/Playback 流程**、eventformat、Lua、存储等打包为独立程序
- 实现 gRPC **客户端**：`StartRecording` + `ReadInputEvents` 订阅事件流、`UploadRecording` 推送完整文件、`PlaybackRecording()` 触发回放、按需 `GetCursorPositionAvailability` / `ReadCursorPosition`

### 阶段四：Lua 与资源路径
- Lua 仅在 app；输出完整资源文件后上传并走 `PlaybackRecording`

---

## 六、部署示例

| 场景 | 设备端部署 | 业务端部署 |
|------|------------|------------|
| 本地测试 | 本机 | 本机 |
| 远程设备 | 被测设备/测试台 | 开发机/CI |
| 多设备 | 每台设备一个 evrp-device | 一个 evrp-app 连接多 device |
| 容器/CI | 需要 device 直通或虚拟 input | 普通容器 |

---

## 七、待澄清事项

1. **Lua 脚本中与设备相关的 API**（如 `keyboard.click`, `mouse.move`）如何映射到「生成事件序列」vs「直接注入」？
2. **eventformat** 的解析逻辑是否需要在两边重复，还是通过共享库/协议统一？
3. **CursorPos** 若仅用于 Playback 时的坐标，是否必须留在 device 端？
4. 是否需要支持 **多 device 并行**（一个 app 控制多台设备）？
5. gRPC **鉴权**（mTLS、token）是否在首版纳入；仅本机 UDS 时是否可省略。
