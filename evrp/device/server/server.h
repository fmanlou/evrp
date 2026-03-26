#pragma once

#include <string>

#include "evrp/device/api/inputlistener.h"

namespace evrp::device::api {

// 启动设备 gRPC 服务并阻塞直到进程结束（实现位于 evrp/device/server 与
// evrp_device_grpc，业务代码无需包含 gRPC 头文件）。
// StartRecording / ReadInputEvents / StopRecording 使用 input_listener；其余 RPC 在
// GrpcInputDeviceService 中暂未实现（返回 UNIMPLEMENTED，Ping 除外）。
// 返回 0 表示正常退出，非 0 表示监听失败等。
int run_device_server(const std::string& listen_address,
                      IInputListener& input_listener);

}  // namespace evrp::device::api
