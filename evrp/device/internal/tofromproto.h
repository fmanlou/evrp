#pragma once

#include <vector>

#include <google/protobuf/repeated_field.h>
#include <google/protobuf/repeated_ptr_field.h>

#include "evrp/device/api/types.h"
#include "evrp/device/v1/types.pb.h"

namespace evrp::device::api {

v1::DeviceKind toProto(DeviceKind k);

DeviceKind fromProto(v1::DeviceKind k);

void fromProto(const google::protobuf::RepeatedField<int>& protoKinds,
               std::vector<DeviceKind>* out);

std::vector<DeviceKind> fromProto(
    const google::protobuf::RepeatedField<int>& protoKinds);

void toProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* protoKinds);

void toProto(const InputEvent& e, v1::InputEvent* p);

void toProto(
    const std::vector<InputEvent>& events,
    google::protobuf::RepeatedPtrField<v1::InputEvent>* out);

void fromProto(const v1::InputEvent& p, InputEvent* e);

void fromProto(
    const google::protobuf::RepeatedPtrField<v1::InputEvent>& protoEvents,
    std::vector<InputEvent>* out);

std::vector<InputEvent> fromProto(
    const google::protobuf::RepeatedPtrField<v1::InputEvent>& protoEvents);

void toProto(const OperationResult& r, v1::OperationResult* p);

}
