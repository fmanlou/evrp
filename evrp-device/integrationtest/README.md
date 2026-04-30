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

- `--device_binary=<生成目录>/evrp-device`
- `--listen_require_valid_event_per_kind=false`

自行命令行运行时若需与 CI 行为一致，请同样传入这两项。以下示例假定构建目录为 `build`，仓库根为当前目录。

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

```bash
./build/evrp_device_integration_test \
  --gtest_filter=DeviceIntegration.<名称> \
  --device_binary=./build/evrp-device \
  --listen_require_valid_event_per_kind=false
```
./build/evrp_device_integration_test \
  --gtest_filter=DeviceIntegration.GetCapabilities \
  --device_binary=./build/evrp-device \
  --listen_require_valid_event_per_kind=false

`<名称>` 与下表一致。

## 用例列表与执行方式

| 用例（GTest / CTest 后缀） | 说明 |
|---------------------------|------|
| `DirectConnect` | `makeClient(target)` 直连会话 |
| `GetCapabilities` | 直连后 `GetCapabilities` |
| `InputListen` | InputListen 流程（与 gflags 如 `--test_input_listen` 等相关） |
| `Playback` | Playback 上传/回放（需设备具备键盘能力时才有实质 RPC） |
| `UdpDiscovery` | `makeClient("")` + UDP 发现；**仅本地 spawn**（传入 `--device_binary`）时执行断言，否则 **GTEST_SKIP** |

**远程设备**（不传 `--device_binary`，改用 `--target=host:port` 或 `--host` + `--port`）时，除 `UdpDiscovery` 会因跳过而不测发现外，其余用例在能连通对端 `evrp-device` 的前提下可同样用 `--gtest_filter` 运行；需自行去掉或改写仅适用于 CI 的 `listen_require_valid_event_per_kind` 等参数。

## GTest 过滤语法提示

- 只跑一个：`--gtest_filter=DeviceIntegration.UdpDiscovery`
- 排除某个：`--gtest_filter=-DeviceIntegration.Playback`
- 说明见 [GoogleTest 高级用法](https://google.github.io/googletest/advanced.html#running-individual-tests)

## 其他常用 gflags

集成测试进程与 `harness.cpp` 中定义一致，例如：`--rpc_wait_ms`、`--log_level`、`--test_udp_discovery`、`--test_input_listen`、`--test_playback` 等。需要时加在可执行文件参数列表末尾即可。
