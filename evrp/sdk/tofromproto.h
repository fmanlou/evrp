#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#include <google/protobuf/repeated_field.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <google/protobuf/struct.pb.h>

#include "evrp/sdk/types.h"
#include "evrp/v1/device/types/common.pb.h"
#include "evrp/v1/sdk/types/common.pb.h"

namespace evrp::sdk {

void fromProto(std::map<std::string, std::any>& out,
               const google::protobuf::Struct& s);

void toProto(const std::map<std::string, std::any>& snap,
             google::protobuf::Struct* out);

evrp::v1::device::DeviceKind toProto(DeviceKind k);

DeviceKind fromProto(evrp::v1::device::DeviceKind k);

void fromProto(const google::protobuf::RepeatedField<int>& protoKinds,
               std::vector<DeviceKind>* out);

std::vector<DeviceKind> fromProto(
    const google::protobuf::RepeatedField<int>& protoKinds);

void toProto(const std::vector<DeviceKind>& kinds,
             google::protobuf::RepeatedField<int>* protoKinds);

void toProto(const InputEvent& e, evrp::v1::device::InputEvent* p);

void toProto(
    const std::vector<InputEvent>& events,
    google::protobuf::RepeatedPtrField<evrp::v1::device::InputEvent>* out);

void fromProto(const evrp::v1::device::InputEvent& p, InputEvent* e);

void fromProto(
    const google::protobuf::RepeatedPtrField<evrp::v1::device::InputEvent>& protoEvents,
    std::vector<InputEvent>* out);

std::vector<InputEvent> fromProto(
    const google::protobuf::RepeatedPtrField<evrp::v1::device::InputEvent>& protoEvents);

void toProto(const StatusCode& r, evrp::v1::sdk::StatusCode* p);

}  // namespace evrp::sdk
