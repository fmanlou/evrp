#pragma once

#include <string>

#include "evrp/device/api/host.h"
#include "evrp/device/api/inputlistener.h"

namespace evrp::device::api {

// 启动设备 gRPC 服务并阻塞直到进程结束（实现位于 evrp_device_grpc 库，业务代码无需包含 gRPC 头文件）。
// StartRecording / ReadInputEvents / StopRecording 使用 input_listener；其余 RPC 使用 host。
// 返回 0 表示正常退出，非 0 表示监听失败等。
int run_device_server(const std::string& listen_address, IDeviceHost& host,
                      IInputListener& input_listener);

}  // namespace evrp::device::api
