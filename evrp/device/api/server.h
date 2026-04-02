#pragma once

#include <string>

#include "evrp/device/api/inputlistener.h"
#include "evrp/device/api/playback.h"

namespace evrp::device::api {

// 启动设备 gRPC 服务并阻塞直到进程结束（实现位于 evrp/device/server 与
// evrp_device_grpc，业务代码无需包含 gRPC 头文件）。
// `InputListenService`：StartRecording / WaitForInputEvent / ReadInputEvents / StopRecording 使用
// input_listener。`PlaybackService`：`Upload` / `Playback` / `Stop` 经由 playback 注入
// GrpcPlaybackService。`InputDeviceService`：光标 /
// Ping 等在 GrpcInputDeviceService 中暂未实现（返回 UNIMPLEMENTED，Ping 除外）。
// 返回 0 表示正常退出，非 0 表示监听失败等。
int runDeviceServer(const std::string& listen_address,
                      IInputListener& input_listener,
                      IPlayback& playback);

}  // namespace evrp::device::api
