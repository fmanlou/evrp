#pragma once

#include <vector>

#include <google/protobuf/repeated_field.h>
#include <google/protobuf/repeated_ptr_field.h>

#include "evrp/device/api/types.h"
#include "evrp/device/v1/device.pb.h"

namespace evrp::device::api {

// ToProto / FromProto：api 类型与 device.proto 互转。带 `*out` / `proto_kinds` 指针的重载为**追加**写入；
// 若需覆盖原内容，调用方须先对 `*out` 或 proto repeated 字段执行 clear。

evrp::device::v1::DeviceKind ToProto(DeviceKind k);

DeviceKind FromProto(evrp::device::v1::DeviceKind k);

void FromProto(const google::protobuf::RepeatedField<int>& proto_kinds,
               std::vector<DeviceKind>* out);

std::vector<DeviceKind> FromProto(
    const google::protobuf::RepeatedField<int>& proto_kinds);

void ToProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* proto_kinds);

void ToProto(const InputEvent& e, evrp::device::v1::InputEvent* p);

void FromProto(const evrp::device::v1::InputEvent& p, InputEvent* e);

void FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events,
    std::vector<InputEvent>* out);

std::vector<InputEvent> FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events);

}  // namespace evrp::device::api
