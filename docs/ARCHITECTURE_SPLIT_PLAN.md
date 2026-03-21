# evrp 架构分离方案

## 目标

将 evrp 拆分为两个独立程序：

1. **设备端程序 (evrp-device)**：运行在配备物理输入设备的机器上，负责输入事件的采集与注入
2. **业务端程序 (evrp-app)**：与设备无关，可运行在任何环境，负责业务逻辑处理

---

## 一、职责划分

### 1.1 设备端程序 (evrp-device)

**运行环境**：必须能访问 `/dev/input/*` 的机器（通常为被测设备或测试台）

**职责**：
- 录制：从 touchpad/touchscreen/mouse/keyboard 采集原始输入事件
- 回放：将事件注入物理设备
- 接收来自业务端的指令与事件流
- 将录制到的事件流上报给业务端

**依赖**：
- Linux input 子系统 (`/dev/input/event*`)
- 可选：X11（光标位置，若需 cursorpos）
- 文件系统（本地临时缓存）
- gRPC 服务端（对外暴露服务）

**不适合承担的职责**：
- 事件格式转换、解析
- Lua 脚本执行（与业务逻辑相关）
- 业务决策、流程编排

---

### 1.2 业务端程序 (evrp-app)

**运行环境**：任意环境（本地、服务器、CI、容器等），无需物理输入设备

**职责**：
- 事件文件的生成、编辑、转换
- Lua 脚本执行（生成事件序列、编排逻辑）
- 事件序列的解析、校验、裁剪
- 录制/回放流程编排与调度
- 存储、版本管理、分析等业务逻辑

**依赖**：
- 文件系统
- gRPC 客户端（连接 evrp-device）
- Lua 解释器
- 不依赖 `/dev/input/*`、X11 等设备接口

---

## 二、通信架构

**协议与接口统一采用 [gRPC](https://grpc.io/)（C++ 使用 `grpc` / `grpc++` 与 `protobuf`）**：IDL 用 `.proto` 定义服务、RPC 与消息类型；传输层由 gRPC 处理（HTTP/2，本地可用 Unix Domain Socket 或 `localhost` TCP）。

```
┌─────────────────────────────────────────────────────────────────┐
│                      evrp-app (业务端)                            │
│  - 解析/生成事件文件                                               │
│  - 执行 Lua 脚本（生成事件序列）                                    │
│  - 流程编排、存储、分析                                             │
│  - gRPC Client                                                    │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                    gRPC (HTTP/2)
                    Unary / Client-Streaming / Server-Streaming / Bidi
                            │
┌───────────────────────────▼─────────────────────────────────────┐
│                    evrp-device (设备端)                           │
│  - gRPC Server                                                    │
│  - Record: 采集事件 → 流式返回 app                                 │
│  - Playback: 接收事件流 → 注入设备                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.1 接口形态（建议，具体以 `.proto` 为准）

| 能力 | gRPC 形态 | 说明 |
|------|-----------|------|
| 开始/停止录制、开始/停止回放、退出等 | Unary RPC | 控制指令，返回简单结果或错误 |
| 录制事件上报 | Server-streaming | device → app，按行或按块推送事件文本/结构化消息 |
| 回放事件下发 | Client-streaming 或 Bidi | app → device 推送事件序列；必要时双向确认 |
| 状态/心跳 | Unary 或 stream | 可选 |

事件载荷可内嵌 **现有 `eventformat` 文本行**（`string` 字段），或拆成结构化 `message` 便于校验；流式 RPC 适合长录制与长回放。

### 2.2 事件载荷与流式传输

- **业务层**仍可与现有 `eventformat` 文本格式对齐，便于解析与落盘。
- **传输层**由 gRPC 的流式 RPC 承载，无需自研分帧协议；大流量时注意 `max_send_message_size` / 背压与流控配置。

---

## 三、模块归属

| 模块 | 设备端 (evrp-device) | 业务端 (evrp-app) |
|------|----------------------|-------------------|
| Record | ✓ | ✗ |
| Playback（注入部分） | ✓ | ✗ |
| InputDevice | ✓ | ✗ |
| InputEventWriter | ✓ | ✗ |
| evdev | ✓ | ✗ |
| CursorPos (X11) | ✓（若需要） | ✗ |
| eventformat (解析) | ✓（解析回放文件） | ✓（解析/生成） |
| Lua + lua_bindings | ✗ | ✓ |
| argparser | 部分 | 部分 |
| logger, filesystem | ✓ | ✓ |

**Lua 脚本的两种形态**：

1. **设备端 Lua**（可选，未来）：在 device 上直接执行，用于简单本地回放
2. **业务端 Lua**：在 app 中执行，生成事件序列后通过网络发送给 device 回放

---

## 四、典型工作流

### 4.1 录制流程

（经 gRPC：Unary 发起录制 + Server-streaming 回传事件）

```
evrp-app                    evrp-device
    |                            |
    |--- record (RPC) ----------->| 开始录制
    |                            | 从设备采集事件
    |<--- 事件流 (gRPC stream) ---| 实时或结束时推送
    |                            |
    | 存储/处理事件文件            |
```

### 4.2 回放流程

（经 gRPC：Unary 或 stream 发起回放 + Client-streaming / Bidi 下发事件）

```
evrp-app                    evrp-device
    |                            |
    | 解析事件文件 / 执行 Lua      |
    | 生成事件序列                 |
    |--- playback + 事件流 (RPC) ->| 注入设备
    |                            |
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

### 阶段一：协议与接口设计（gRPC）
- 编写 `.proto`：服务名、RPC 列表、请求/响应消息、流式语义（Unary / streaming）
- 选用 `grpc_cpp_plugin` 生成 C++ stub 与 service 基类；CMake 集成 `find_package(gRPC)` 或 `FetchContent`/`add_subdirectory` 引入依赖
- 约定监听地址（如 `unix:///path/evrp.sock` 或 `host:port`）、TLS（若跨机）与消息大小限制

### 阶段二：evrp-device 抽取
- 将 Record、Playback、InputDevice、InputEventWriter 等打包为独立可执行程序
- 实现 gRPC **服务端**：实现 `.proto` 中定义的 service，在 RPC 内驱动录制/回放与流式收发

### 阶段三：evrp-app 抽取
- 将 eventformat 解析、Lua 执行、业务逻辑打包为独立程序
- 实现 gRPC **客户端**：调用 device 上的 RPC，发起控制与流式事件交互

### 阶段四：Lua 解耦
- 业务端 Lua：生成事件序列，不直接调用 InputEventWriter
- 通过 gRPC 流式 RPC 将事件序列发往 device 执行注入

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
