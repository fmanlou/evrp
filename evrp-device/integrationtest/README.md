# evrp-device 集成测试

源码位于本目录；可执行文件由 CMake 生成：`evrp_device_integration_test`（依赖已构建的 `evrp-device`）。

## 构建

在仓库根目录配置并编译（示例）：

```bash
cmake -S . -B build
cmake --build build --target evrp_device_integration_test
```

## CI 默认参数

`CMakeLists.txt` 里通过 `gtest_discover_tests` 为每个用例附加：

- `--device_binary=<生成目录>/evrp-device`（由 CMake 自动拉起本机 `evrp-device`）
- `--listen_require_valid_event_per_kind=false`

本地**自行先启动** `evrp-device` 时不必传 `--device_binary`。

- **只跑 `UdpDiscovery`**：不必传 `--target` / `--host`，只要局域网内有一台 `evrp-device` 的 `--discovery_port` 与测试进程一致（默认 `53508`），`makeClient("")` 能发现即可。
- **跑直连类用例**（`DirectConnect`、`GetCapabilities` 等）：需再加 `--target=host:port` 或 `--host` + `--port`，与 device 的 `-listen` 一致。

自行命令行跑 CI 同款行为时，可继续传入上述两项。以下示例假定构建目录为 `build`，仓库根为当前目录。

## 运行全部用例（CTest）

```bash
ctest --test-dir build -R DeviceIntegration --output-on-failure
```

## 单独运行某个用例

每个用例在 CTest 中的名称形如 `DeviceIntegration.<用例名>`。可用 **CTest 按名筛选**，或直接跑可执行文件并设置 **GTest filter**。

### 用 CTest（推荐）

将 `<名称>` 换成下表中的 CTest 名称后缀（例如 `UdpDiscovery`）。

```bash
ctest --test-dir build -R "DeviceIntegration.<名称>" --output-on-failure
```

示例（只跑 UDP 发现）：

```bash
ctest --test-dir build -R "DeviceIntegration.UdpDiscovery" --output-on-failure
```

### 直接运行可执行文件

**只测 UDP 发现**（不传 `--target`，需网内已有 `evrp-device`，发现端口一致即可）：

```bash
./build/evrp_device_integration_test \
  --gtest_filter=DeviceIntegration.UdpDiscovery
```

**跑全部或含直连的用例**：需指定 gRPC 地址。先手动启动 device（`-listen` 与下面 `--target` 一致；发现端口与 `--discovery_port` 一致时可省略该参数，默认即可）：

```bash
./build/evrp-device -listen 0.0.0.0:50051 --log_level=info
```

```bash
./build/evrp_device_integration_test \
  --gtest_filter=DeviceIntegration.<名称> \
  --target=127.0.0.1:50051 \
  --listen_require_valid_event_per_kind=false
```

等价写法：`--host=127.0.0.1 --port=50051`。若需与 CI 一样由测试进程 fork 子进程启动 `evrp-device`，使用 `--device_binary=./build/evrp-device` 且不要传 `--target`。

`<名称>` 与下表一致。

## 用例列表与执行方式

| 用例（GTest / CTest 后缀） | 说明 |
|---------------------------|------|
| `DirectConnect` | `makeClient(target)` 直连会话 |
| `GetCapabilities` | 直连后 `GetCapabilities` |
| `InputListen` | InputListen 流程（与 gflags 如 `--test_input_listen` 等相关） |
| `Playback` | Playback 上传/回放（需设备具备键盘能力时才有实质 RPC） |
| `UdpDiscovery` | `makeClient("")` + UDP 发现；与对端 **`--discovery_port` 一致即可**（默认即可），**不必**传 `--target`。若同时传 `--target`，则额外校验发现地址与之一致 |

**不传 `--target` 时**，除 `UdpDiscovery` 外其余用例会 **GTEST_SKIP**（它们测的是直连 `makeClient(host:port)`）。要跑满套件请用 `--device_binary` 或 `--target` / `--host`+`--port`。

## GTest 过滤语法提示

- 只跑一个：`--gtest_filter=DeviceIntegration.UdpDiscovery`
- 排除某个：`--gtest_filter=-DeviceIntegration.Playback`
- 说明见 [GoogleTest 高级用法](https://google.github.io/googletest/advanced.html#running-individual-tests)

## 其他常用 gflags

集成测试进程与 `harness.cpp` 中定义一致，例如：`--rpc_wait_ms`、`--log_level`、`--test_udp_discovery`、`--test_input_listen`、`--test_playback` 等。需要时加在可执行文件参数列表末尾即可。
