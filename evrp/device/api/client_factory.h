#pragma once

#include <memory>
#include <string>

#include "evrp/device/api/client.h"

namespace evrp {
namespace device {
namespace api {

// 连接已运行的设备端（实现使用 gRPC，业务代码无需包含 gRPC / proto 头文件）。
// target 形如 "127.0.0.1:50051"（具体格式由 gRPC 实现约定）。
std::unique_ptr<IDeviceClient> ConnectDeviceClient(const std::string& target);

}  // namespace api
}  // namespace device
}  // namespace evrp
