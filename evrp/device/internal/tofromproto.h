#pragma once

#include <vector>

#include <google/protobuf/repeated_field.h>
#include <google/protobuf/repeated_ptr_field.h>

#include "evrp/device/api/types.h"
#include "evrp/device/v1/types.pb.h"

namespace evrp::device::api {

evrp::device::v1::DeviceKind ToProto(DeviceKind k);

DeviceKind FromProto(evrp::device::v1::DeviceKind k);

void FromProto(const google::protobuf::RepeatedField<int>& proto_kinds,
               std::vector<DeviceKind>* out);

std::vector<DeviceKind> FromProto(
    const google::protobuf::RepeatedField<int>& proto_kinds);

void ToProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* proto_kinds);

void ToProto(const InputEvent& e, evrp::device::v1::InputEvent* p);

void ToProto(
    const std::vector<InputEvent>& events,
    google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>* out);

void FromProto(const evrp::device::v1::InputEvent& p, InputEvent* e);

void FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events,
    std::vector<InputEvent>* out);

std::vector<InputEvent> FromProto(
    const google::protobuf::RepeatedPtrField<evrp::device::v1::InputEvent>&
        proto_events);

void ToProto(const OperationResult& r, evrp::device::v1::OperationResult* p);

}
